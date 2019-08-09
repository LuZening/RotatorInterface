#include "motor.h"
void init_motor(struct Motor* pmotor, int pin_break, int pin_motor_N, int pin_motor_P)
{
    pmotor->pin_break = pin_break;
    pmotor->pin_motor_N = pin_motor_N;
    pmotor->pin_motor_P = pin_motor_P;
    pmotor->is_break_released = false;
    pmotor->status = MOT_IDLE;
    engage_break(pmotor);
}

void engage_break(struct Motor* pmotor)
{
    delay(BREAK_ENGAGE_DELAY);
    SET_PIN(pmotor->pin_break, MOTOR_OFF);
    pmotor->is_break_released = false;
}

void release_break(struct Motor* pmotor)
{
    SET_PIN(pmotor->pin_break, MOTOR_ON);
    delay(BREAK_RELEASE_DELAY);
    pmotor->is_break_released = true;
    /*
    if(pmotor->timeout_break_release <= 0)
    {
        SET_PIN(pmotor->pin_break, MOTOR_ON);
        pmotor->timeout_break_release = TIMEOUT_BREAK_RELEASE;
    }
    else
    {
        pmotor->timeout_break_release -= INTERVAL_HANDLE_MOTOR;
        if(pmotor->timeout_break_release <= 0)
        {
            pmotor->is_break_released = true;
        }
    }
    */
}

bool is_break_released(struct Motor* pmotor)
{
    return (pmotor->is_break_released);
}

void run_CCW(struct Motor* pmotor)
{
    pmotor->status = MOT_CCW; 
    while(!is_break_released(pmotor))
    {
        release_break(pmotor);
    }
    SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
    SET_PIN(pmotor->pin_motor_N, MOTOR_ON);
}

void run_CW(struct Motor* pmotor)
{
    pmotor->status = MOT_CW;
    while(!is_break_released(pmotor))
    {
        release_break(pmotor);
    }
    SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
    SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
}

bool stop_motor(struct Motor* pmotor)
{
    SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
    SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
    engage_break(pmotor);
    pmotor->status = MOT_IDLE;
    return true;
}

void accelerate(struct Motor* pmotor){};