#include "RotTask.h"
#include "RotSensor.h"
#include "motor.h"
#include "main.h"
#include "Config.h"
#include "Lib595.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// Motors (extended by 74595)
// use TRIAC+RELAY output board
#ifdef OUTPUT_BOARD_TRIAC_RELAY
const int MOT_BRAKE_PINS[] = {TO_595_PIN_NUMBER(6U), 0};
const int MOT_POWER_PINS[] = {TO_595_PIN_NUMBER(2U), TO_595_PIN_NUMBER(4U)};
const int MOT_DIRECTION_PINS[] = {TO_595_PIN_NUMBER(3U), TO_595_PIN_NUMBER(5U)};
const int MOT_PWM_A_PINS[] = {13, 14};
const int MOT_PWM_B_PINS[] = {13, 14};
#endif

QueueHandle_t qRotExecutorTasks[N_MOT_OUTPUTS];
TaskHandle_t taskRotExecutor[N_MOT_OUTPUTS];
Motor arrMotors[N_MOT_OUTPUTS];

void task_rot_executor(void* args);

void task_rot_tasks(void* args)
{
    /* setup */
    char bufName[16];
    for(int i = 0; i < cfg.N_MOTs; ++i)
    {
        Motor *pMot = &(arrMotors[i]);
        init_motor(pMot, cfg.motDriveModes[i], MOT_BRAKE_PINS[i], MOT_DIRECTION_PINS[i], MOT_POWER_PINS[i], MOT_PWM_A_PINS[i], MOT_PWM_B_PINS[i], MCPWM_UNIT_0);
        snprintf(bufName, sizeof(bufName)-1, "rot_exe.%2ud", i+1);
        // create a mailbox queue for each motor executor
        qRotExecutorTasks[i] = xQueueCreate(1, sizeof(RotTask_t));
        // create a task for each motor for execution
        xTaskCreate(task_rot_executor,  bufName, 512, (qRotExecutorTasks[i]), 1, &(taskRotExecutor[i]));
    }
    /* main loop BEGIN */
    RotTask_t rottask = {.type = ROT_TASK_NULL}; // current task
    RotTask_t rottask_new;
    while(1)
    {
        // take a task from task queue
        if(xQueueReceive(qRotTasks, &rottask_new, 0) == pdPASS )
        {
            int iMot = rottask_new.motnum - 1;
            if(!rottask_new.is_executed && rottask_new.motnum >= 1 && rottask_new.motnum <= N_MOT_OUTPUTS)
            {
                Motor* pMot = &(arrMotors[iMot]);
                xQueueOverwrite(qRotExecutorTasks[iMot], &rottask_new);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100U));
    }
    /* main loop END */
}


// Rotator executor task
// @params qRotTaskExecutor
void task_rot_executor(void* args)
{
    /* setup */
    QueueHandle_t qTaskToExecute = (QueueHandle_t)args;
    RotTask_t rottask_prev;
    RotTask_t rottask_new;
    bool isNewTaskReceived = false;
    init_task(&(rottask_prev));
    /* super loop */
    while(1)
    {
        if(!isNewTaskReceived)
            xQueueReceive(qRotExecutorTasks, &rottask_new, 0); // waiting for a new task
        rottask_new.is_executed = false;
        int iMot = rottask_new.motnum - 1;
        Motor *pMot = &(arrMotors[iMot]);
        uint32_t nManualTaskLifetimeRemain = 0;
        int nTarget = 0;
        // curMotDir is the current rotating direction of the motor, for TARGET task
        int curMotDir = 0; // CW: 1, CCW: -1
        /* switch task type*/
        switch(rottask_new.type)
        {
            case ROT_TASK_NULL:
            // stop current tasks if the previous task is not NULL
            if(rottask_prev.type != ROT_TASK_NULL)
            {
                stop_motor(pMot);
                nManualTaskLifetimeRemain = 0;
                isNewTaskReceived = true;
                vTaskDelay(pdMS_TO_TICKS(pMot->brake_engage_defer));
                engage_brake(pMot);
            }
            break;
            case ROT_TASK_MANUAL:
            if(rottask_prev.type  == ROT_TASK_NULL) 
            {
                // start a new manual task
                release_brake(pMot);
                motor_set_speed(pMot, rottask_new.n_speed);
                if(rottask_new.n_to > 0)
                {
                    run_CW(pMot);
                    nManualTaskLifetimeRemain = rottask_new.n_to;
                    rottask_new.is_executed = true;
                }
                else if(rottask_new.n_to < 0)
                {
                    run_CCW(pMot);
                    nManualTaskLifetimeRemain = -rottask_new.n_to;
                    rottask_new.is_executed = true;
                }
            }
            else if(rottask_prev.type == ROT_TASK_MANUAL)
            {
                // direction differs, stop the current task, rest for a while, and turn a round
                if((rottask_prev.n_to > 0 && rottask_new.n_to <= 0) 
                    || (rottask_prev.n_to < 0 && rottask_new.n_to >= 0))
                {
                    stop_motor(pMot);
                    vTaskDelay(pdMS_TO_TICKS(cfg.brake_engage_defer[iMot]*2));
                    motor_set_speed(pMot, rottask_new.n_speed);
                    if(rottask_new.n_to > 0)
                    {
                        run_CW(pMot);
                        nManualTaskLifetimeRemain = rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                    else if(rottask_new.n_to < 0)
                    {
                        run_CCW(pMot);
                        nManualTaskLifetimeRemain = -rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                }
                // direction aggress, extend the duration of the task
                else
                {
                    motor_set_speed(pMot, rottask_new.n_speed);
                    if(rottask_new.n_to > 0)
                    {
                        nManualTaskLifetimeRemain = rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                    else if(rottask_new.n_to < 0)
                    {
                        nManualTaskLifetimeRemain = -rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                }
            }
            else if(rottask_prev.type == ROT_TASK_TARGET)
            {
                // if the  new MANUAL task's direction differs from previous task's, stop the previous task, take a rest and turn around
                if((curMotDir > 0 && rottask_new.n_to <= 0) || (curMotDir < 0 && rottask_new.n_to >= 0) )
                {
                    stop_motor(pMot);
                    vTaskDelay(2 * pdMS_TO_TICKS(cfg.brake_engage_defer[iMot]));
                    motor_set_speed(pMot, rottask_new.n_speed);
                    if(rottask_new.n_to > 0)
                    {
                        run_CW(pMot);
                        nManualTaskLifetimeRemain = rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                    else if(rottask_new.n_to < 0)
                    {
                        run_CCW(pMot);
                        nManualTaskLifetimeRemain = -rottask_new.n_to;
                        rottask_new.is_executed = true;
                    }
                }
                // direction aggress, only change motor direction and continue the motor rotation
                else
                {
                    motor_set_speed(pMot, rottask_new.n_speed);
                    // if motor is not rotating, let it rotate  
                    if(pMot->status == MOT_IDLE)
                    {
                        if(rottask_new.n_to > 0)
                        {
                            run_CW(pMot);
                            nManualTaskLifetimeRemain = rottask_new.n_to;
                            rottask_new.is_executed = true;
                        }
                        else if(rottask_new.n_to < 0)
                        {
                            run_CCW(pMot);
                            nManualTaskLifetimeRemain = -rottask_new.n_to;
                            rottask_new.is_executed = true;
                        }
                    }
                    // if mot is rotating, just extend the lifetime
                    else
                    {
                        if(rottask_new.n_to > 0)
                        {
                            nManualTaskLifetimeRemain = rottask_new.n_to;
                            rottask_new.is_executed = true;
                        }
                        else if(rottask_new.n_to < 0)
                        {
                            nManualTaskLifetimeRemain = -rottask_new.n_to;
                            rottask_new.is_executed = true;
                        }
                    }
                }
            }
            break;
            case ROT_TASK_TARGET:
            // TODO: implement TARGET task executor
            // the new task is TARGET, the previous task is NULL, then rotate the motor
            if(rottask_prev.type == ROT_TASK_NULL)
            {
                // calculate the target direction

            }
            break;
        }
        isNewTaskReceived = false; // clear the indicator that new task has been receved
        // if the new task is executed successfully, replace the previous task by the new one
        if(rottask_new.is_executed)
            rottask_prev = rottask_new; 
        /* mantain the current task BEGIN */
        switch (rottask_prev.type)
        {
        /* the current task executing is MANUAL */
        case ROT_TASK_MANUAL:
            /* maintain the manual task until lifetime runs out or received new task */
            if(xQueueReceive(qRotExecutorTasks, &rottask_new, pdMS_TO_TICKS(nManualTaskLifetimeRemain))
                == errQUEUE_EMPTY)
            {
                    // timed out, manual task finished
                    nManualTaskLifetimeRemain = 0;
                    rottask_prev.type = ROT_TASK_NULL;
                    // stop the motor
                    stop_motor(pMot);
                    // engage brake
                    vTaskDelay(pdMS_TO_TICKS(pMot->brake_engage_defer));
                    engage_brake(pMot);
            }
            else // received new task, let the task switching (previous) to handle the job
            {
                isNewTaskReceived = true;
            }
            break;
        /* the current task executing is TARGET */
        case ROT_TASK_TARGET:
            // TODO: implement target task support

        default:
            break;
        }
        /* mantain the current task END */

    }
}