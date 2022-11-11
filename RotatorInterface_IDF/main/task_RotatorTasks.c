#include "RotTask.h"
#include "RotSensor.h"
#include "motor.h"
#include "main.h"
#include "Config.h"
#include "Lib595.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "webserver.h"
#include "task_display.h"
#define tag "RotTask"


#define MIN(a,b) ((a < b)?(a):(b))
// Motors (extended by 74HC595)
// use TRIAC+RELAY output board
#ifdef OUTPUT_BOARD_TRIAC_RELAY
const int MOT_BRAKE_PINS[] = {TO_595_PIN_NUMBER(6U), 0};
const int MOT_POWER_PINS[] = {TO_595_PIN_NUMBER(2U), TO_595_PIN_NUMBER(4U)};
const int MOT_DIRECTION_PINS[] = {TO_595_PIN_NUMBER(3U), TO_595_PIN_NUMBER(5U)};
const int MOT_PWM_A_PINS[] = {13, 14};
const int MOT_PWM_B_PINS[] = {0, 0};
#endif

#define TARGET_TASK_THRESHOLD_DEGREE 5
#define TARGET_TASK_MAX_LIFETIME_MS 120000
#define TARGET_TASK_ANTI_NOISE_MS 300


QueueHandle_t qRotExecutorTasks[N_MOT_OUTPUTS];
TaskHandle_t taskRotExecutor[N_MOT_OUTPUTS];
TaskHandle_t taskRotIdleMonitors[N_MOT_OUTPUTS];
SemaphoreHandle_t mtxMotors[N_MOT_OUTPUTS];
Motor arrMotors[N_MOT_OUTPUTS];
int MotIDs[N_MOT_OUTPUTS] = {0, 1};

const int TASK_ROT_EXECUTOR_TICK_PERIOD_MS = 50;
const int TIME_BEFORE_BRAKE_ENGAGE_WHEN_IDLE = 3000;
void task_rot_executor(void* args);
// counting the idle time of the motor and engage brake.
// args: int motor id 0,1,...,N_MOTs-1
// void task_rot_idle_monitor(void* args); 

// ENTRY
void task_rot_tasks(void* args)
{
    /* setup */
    
    char bufName[16];
    for(int i = 0; i < cfg.N_MOTs; ++i)
    {
        Motor *pMot = &(arrMotors[i]);
        init_motor(pMot, cfg.motDriveModes[i], MOT_BRAKE_PINS[i], MOT_DIRECTION_PINS[i], MOT_POWER_PINS[i], MOT_PWM_A_PINS[i], MOT_PWM_B_PINS[i], MCPWM_UNIT_0,
                cfg.brake_engage_defer[i], cfg.soft_start_duration[i]);
        snprintf(bufName, sizeof(bufName)-1, "rot_exe.%1u", i+1);
        // create a mailbox queue for each motor executor
        qRotExecutorTasks[i] = xQueueCreate(2, sizeof(RotTask_t));
        mtxMotors[i] = xSemaphoreCreateMutex();
        // create a task for each motor for execution
        xTaskCreate(task_rot_executor,  bufName, 2048 + 256, &(MotIDs[i]), 5, &(taskRotExecutor[i]));
    }
    ESP_LOGD(tag, "rot_task initialized");
    /* main loop BEGIN */
    RotTask_t rottask_new;
    while(1)
    {
        // take a task from task queue
        if(xQueueReceive(qRotTasks, &rottask_new, portMAX_DELAY) == pdPASS )
        {
            int iMot = rottask_new.motnum - 1;
            ESP_LOGD(tag, "task received on mot.%1u", iMot+1);
            if(!rottask_new.is_executed && rottask_new.motnum >= 1 && rottask_new.motnum <= N_MOT_OUTPUTS)
            {
                xQueueSendToBack(qRotExecutorTasks[iMot], &rottask_new, portMAX_DELAY);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100U));
    }
    /* main loop END */
}


// Rotator executor task
// @params id of motor
void task_rot_executor(void* args)
{
    /* setup */
    int iMot;
    iMot = *(int*)args;
    Motor *pMot = &(arrMotors[iMot]);
    ESP_LOGD("task_rot_exe", "task_rot_exe.%1u created", iMot + 1);
    QueueHandle_t qTaskToExecute = qRotExecutorTasks[iMot];
    SemaphoreHandle_t mtxMot = mtxMotors[iMot];

    char sTaskIdleName[16];
    snprintf(sTaskIdleName, sizeof(sTaskIdleName) - 1, "rot_idle.%1u", iMot + 1);
    // create the monitor task for idle monitoring, which decides when to engage brake
    // using block mode
    // xTaskCreate(task_rot_idle_monitor, sTaskIdleName, 2048, &MotIDs[iMot], 2, &(taskRotIdleMonitors[iMot]));
    RotTask_t rottask_prev;
    init_task(&(rottask_prev));
    RotTask_t rottask_new;
    bool isNewTaskReceived = false;
    static int32_t nManualTaskLifetimeRemain = 0;
    static int8_t nDirTargetTask = 0;
    static int16_t nTargetTaskToDegree = 0;
    static int16_t nTargetTaskAntiNoiseCnt_ms = 0;
    static int32_t nTargetTaskLifetimeRemain = 0;
    static int32_t nIdleTimeCounter = 0;
    TickType_t prevWakeTime = xTaskGetTickCount();
    TickType_t prevManualLifeTimeUpdateTick = prevWakeTime;
    static int32_t nSignedSpeedPrev = 0;
    char msg[128];
    /* super loop */
    while(1)
    {
        /* using state machine */
        /* motor state machine states
        *   0 = braked
        *   1 = idle, with brake released
        *   2 = starting
        *   3 = running
        * */
        // check for new task, no wait
        if(xQueueReceive(qTaskToExecute, &rottask_new, pdMS_TO_TICKS(TASK_ROT_EXECUTOR_TICK_PERIOD_MS)) == pdPASS)
        {
            isNewTaskReceived = true;
            // pre process task contents
            if(rottask_new.n_speed > 100)
                rottask_new.n_speed = 100;
            if(rottask_new.n_to > 90000)
                rottask_new.n_to = 90000;
            else if(rottask_new.n_to < -90000)
                rottask_new.n_to = -90000;
            ESP_LOGD("task_exe", "task_exe.%u new task received type:%u to:%d speed:%u", rottask_new.motnum, rottask_new.type, rottask_new.n_to, rottask_new.n_speed);
            // release brake if task is valid
            if(rottask_new.type != ROT_TASK_NULL && rottask_new.n_to != 0 && rottask_new.n_speed != 0)
            {
                if(!pMot->is_brake_released)
                    release_brake(pMot);
            }
            prevWakeTime = xTaskGetTickCount();
        }
        else
            isNewTaskReceived = false;
        xSemaphoreTake(mtxMot, portMAX_DELAY);
        // recieved new task, change motor targets
        int16_t degNow = get_current_degree_value(iMot); // current degree
        if(isNewTaskReceived)
        {
            if(rottask_new.type == ROT_TASK_NULL) // stop motor
            {
                motor_set_target_speed_and_direction(pMot, 0, 0);
                rottask_new.is_executed = true;
                nManualTaskLifetimeRemain = 0;
                int len = snprintf(msg, sizeof(msg), "task_exe.%u all tasks stopped", iMot + 1);
                websocket_broadcast("/ws", msg, MIN(len, sizeof(msg)));
            }
            else if(rottask_new.type == ROT_TASK_MANUAL) // manual task
            {
                if(rottask_new.n_to > 0)
                {
                    nManualTaskLifetimeRemain = rottask_new.n_to;
                    motor_set_target_speed_and_direction(pMot, rottask_new.n_speed, MOT_CW);
                    rottask_new.is_executed = true;
                }
                else if(rottask_new.n_to < 0)
                {
                    nManualTaskLifetimeRemain = -rottask_new.n_to;
                    motor_set_target_speed_and_direction(pMot, rottask_new.n_speed, MOT_CCW);
                    rottask_new.is_executed = true;
                }
                else // stop
                {
                    motor_set_target_speed_and_direction(pMot, 0, 0);
                }
            }
            else if(rottask_new.type == ROT_TASK_TARGET) // TODO: implement target task
            {
                if(degNow != LOOP_BUFFER_INVALID_VALUE)
                {
                    int16_t diff;
                    // regulate to degree to be within 360deg difference with degNow
                    nTargetTaskToDegree = regulate_degree_to_shortest_path(rottask_new.n_to, degNow, 360);
                    diff = nTargetTaskToDegree - degNow;
                    if(diff > 0) // CW
                    {
                        motor_set_target_speed_and_direction(pMot, rottask_new.n_speed, MOT_CW);
                        rottask_new.is_executed = true;
                        nDirTargetTask = MOT_CW;
                        nTargetTaskLifetimeRemain = TARGET_TASK_MAX_LIFETIME_MS;
                    }
                    else if(diff < 0)
                    {
                        motor_set_target_speed_and_direction(pMot, rottask_new.n_speed, MOT_CCW);
                        rottask_new.is_executed = true;
                        nDirTargetTask = MOT_CCW;
                        nTargetTaskLifetimeRemain = TARGET_TASK_MAX_LIFETIME_MS;
                    }
                }
                nTargetTaskAntiNoiseCnt_ms = 0;
            }
            prevManualLifeTimeUpdateTick = xTaskGetTickCount();
            if(rottask_new.is_executed) rottask_prev = rottask_new;
        }
        else // not received new task, just maintain the current task
        {
            if(rottask_prev.type == ROT_TASK_MANUAL)
            {
                if(nManualTaskLifetimeRemain <= 0) // time's up, terminate the task and stop the motor
                {
                    motor_set_target_speed_and_direction(pMot, 0, 0);
                    rottask_prev.type = ROT_TASK_NULL;
                    int len = snprintf(msg, sizeof(msg), "task_exe.%u manual task finished", rottask_new.motnum);
                    websocket_broadcast("/ws", msg, MIN(len, sizeof(msg)));
                }
                else
                {
                    TickType_t tickNow = xTaskGetTickCount();
                    int msDiff = (tickNow - prevManualLifeTimeUpdateTick) * portTICK_RATE_MS;
                    prevManualLifeTimeUpdateTick = tickNow;
                    nManualTaskLifetimeRemain -= msDiff;
                }
            }
            else if(rottask_prev.type == ROT_TASK_NULL)
            {
            }
            // TODO: 
            else if(rottask_prev.type == ROT_TASK_TARGET)
            {
                TickType_t tickNow = xTaskGetTickCount();
                int msDiff = (tickNow - prevManualLifeTimeUpdateTick) * portTICK_RATE_MS;
                prevManualLifeTimeUpdateTick = tickNow;
                // target task termination condition
                // anti-noise mechanism: the condition is always satisfied within an interval
                if((nDirTargetTask == MOT_CW && (nTargetTaskToDegree - degNow) <= TARGET_TASK_THRESHOLD_DEGREE) 
                    || (nDirTargetTask == MOT_CCW && (degNow - nTargetTaskToDegree) <= TARGET_TASK_THRESHOLD_DEGREE))
                {
                    if(nTargetTaskAntiNoiseCnt_ms >= TARGET_TASK_ANTI_NOISE_MS)
                    {
                        motor_set_target_speed_and_direction(pMot, 0, 0);
                        rottask_prev.type = ROT_TASK_NULL;
                        nDirTargetTask = 0;
                        nTargetTaskAntiNoiseCnt_ms = 0;
                        int len = snprintf(msg, sizeof(msg), "task_exe.%u target task to=%d finished", iMot + 1, nTargetTaskToDegree);
                        websocket_broadcast("/ws", msg, MIN(len, sizeof(msg)));
                    }
                    else
                    {
                        nTargetTaskAntiNoiseCnt_ms += msDiff;
                    }
                }
                // target task time out
                else if(nTargetTaskLifetimeRemain <= 0)
                {
                    nDirTargetTask = 0;
                    motor_set_target_speed_and_direction(pMot, 0, 0);
                    rottask_prev.type = ROT_TASK_NULL;
                    nTargetTaskLifetimeRemain = 0;
                    int len = snprintf(msg, sizeof(msg), "task_exe.%u target task to=%d timeout", iMot + 1, nTargetTaskToDegree);
                    websocket_broadcast("/ws", msg, MIN(len, sizeof(msg)));
                }
                // if target is not yet reached, countdown the lifetime
                else
                {
                    nTargetTaskLifetimeRemain -= msDiff;
                    nTargetTaskAntiNoiseCnt_ms = 0;
                }
            }
        }

        TickType_t tickNow = xTaskGetTickCount();
        int msAfterPrevWake =  (tickNow - prevWakeTime) * portTICK_RATE_MS;
        prevWakeTime = tickNow;
        int32_t nSignedSpeedNew = motor_speed_update_handle(pMot, msAfterPrevWake);
        // update motor speed on screen
        if (nSignedSpeedNew != nSignedSpeedPrev)
        {
            switch(iMot)
            {
                case 0:
                display_update_azu_mot_speed(nSignedSpeedNew);
                break;
                case 1:
                display_update_elv_mot_speed(nSignedSpeedNew);
                break;
                default:
                break;
            }
            nSignedSpeedPrev = nSignedSpeedNew;
        }
        // idle time counter
        if(rottask_prev.type == ROT_TASK_NULL && pMot->motStatus == MOT_IDLE_WITH_BRAKE_RELEASED)
        {
            nIdleTimeCounter += msAfterPrevWake;
            if(nIdleTimeCounter >= TIME_BEFORE_BRAKE_ENGAGE_WHEN_IDLE)
            {
                engage_brake(pMot);
                nIdleTimeCounter = 0;
            }
        }
        else
        {
            nIdleTimeCounter = 0;
        }
        xSemaphoreGive(mtxMot);
        // vTaskDelayUntil(&prevWakeTime, pdMS_TO_TICKS(TASK_ROT_EXECUTOR_TICK_PERIOD_MS));
        /* using block mode*/
        #if false
        {
        if(!isNewTaskReceived)
        {
            xQueueReceive(qTaskToExecute, &rottask_new, portMAX_DELAY); // waiting for a new task
            ESP_LOGI("task_exe", "task_exe.%u new task received type:%u to:%d speed:%u", rottask_new.motnum, rottask_new.type, rottask_new.n_to, rottask_new.n_speed);
            rottask_new.is_executed = false;
            isNewTaskReceived = true;
        }
        int nTarget = 0;
        // curMotDir is the current rotating direction of the motor, for TARGET task
        int curMotDir = 0; // CW: 1, CCW: -1
        /* switch task type*/
        xSemaphoreTake(mtxMot, portMAX_DELAY);
        switch(rottask_new.type)
        {
            case ROT_TASK_NULL:
            // stop current tasks if the previous task is not NULL
            if(rottask_prev.type != ROT_TASK_NULL)
            {
                nManualTaskLifetimeRemain = 0;
                isNewTaskReceived = true;
                stop_motor(pMot);
                // vTaskDelay(pdMS_TO_TICKS(pMot->brake_engage_defer));
                // engage_brake(pMot);
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
        xSemaphoreGive(mtxMot);
        if (isNewTaskReceived)
        {
            isNewTaskReceived = false; // clear the indicator that new task has been receved
            // if the new task is executed successfully, replace the previous task by the new one
            if(rottask_new.is_executed)
                rottask_prev = rottask_new; 
        }
        /////////////////////////////////////////////////////
        /* mantain the current task BEGIN */
        switch (rottask_prev.type)
        {
        /* the current task executing is MANUAL */
        case ROT_TASK_MANUAL:
            /* maintain the manual task until lifetime runs out or received new task */
            if(xQueueReceive(qTaskToExecute, &rottask_new, pdMS_TO_TICKS(nManualTaskLifetimeRemain))
                == errQUEUE_EMPTY)
            {
                // timed out, manual task finished
                nManualTaskLifetimeRemain = 0;
                rottask_prev.type = ROT_TASK_NULL;
                // stop the motor
                xSemaphoreTake(mtxMot, portMAX_DELAY);
                stop_motor(pMot);
                xSemaphoreGive(mtxMot);
            }
            else // received new task, let the task switching (previous) to handle the job
            {
                isNewTaskReceived = true;
                ESP_LOGI("task_exe", "task_exe.%u shift to incoming task type:%u to:%d speed:%u", rottask_new.motnum, rottask_new.type, rottask_new.n_to, rottask_new.n_speed);
            }
            break;
        /* the current task executing is TARGET */
        case ROT_TASK_TARGET:
            // TODO: implement target task support

        default:
            break;
        }
        /* mantain the current task END */
        /////////////////////////////////////////////////////////////////
        }
        #endif
    }
}


// void task_rot_idle_monitor(void* args)
// {
//     uint32_t nMotIdleCounter = 0;
//     SemaphoreHandle_t mtxMot;
//     int iMot;
//     iMot = *(int*)args;
//     ESP_LOGI("task_rot_idle", "task_rot_idle.%1u created", iMot);
//     Motor* pMot = &(arrMotors[iMot]);
//     mtxMot = mtxMotors[iMot];
//     while(1)
//     {
//         /* Count how long the motor has been idle */
//         xSemaphoreTake(mtxMot, portMAX_DELAY);
//         if(pMot->status == MOT_IDLE && pMot->is_brake_released)
//         {
//             if((nMotIdleCounter++) >= pdMS_TO_TICKS(3 * cfg.brake_engage_defer[iMot]))
//             {
//                 ESP_LOGI("task_rot_idle", "mot %1u brake engaged after %u ticks of idle", iMot, nMotIdleCounter);
//                 nMotIdleCounter = 0;
//                 engage_brake(pMot);
//             }
//         }
//         else
//         {
//             nMotIdleCounter = 0;
//         }
//         xSemaphoreGive(mtxMot);
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }
