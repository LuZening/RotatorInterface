#include "motor.h"
#include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

#include "freertos/task.h"
#include "freertos/queue.h"

static void DELAY_MS(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}


void init_motor(Motor* pmotor, mot_drive_mode_t mode_drive, int pin_brake, int pin_motor_N, int pin_motor_P, int pin_PWM_A, int pin_PWM_B, mcpwm_unit_t mcpwm_unit)
{
    pmotor->mode_drive = mode_drive;
    pmotor->pin_brake = pin_brake;
    pmotor->pin_motor_N = pin_motor_N;
    pmotor->pin_motor_P = pin_motor_P;
    pmotor->is_brake_released = false;
    pmotor->status = MOT_IDLE;
    pmotor->target_speed = 0;
    pmotor->mcpwm_unit = mcpwm_unit;
    pmotor->pin_PWM_A = pin_PWM_A;
    pmotor->pin_PWM_B = pin_PWM_B;
    // if motor uses PWM speed control, then initialize PWM device
    if(pin_PWM_A != MOT_PIN_NONE)
    {
        /* Init PWM device BEGIN */
        mcpwm_config_t mcpwm_config = {
            .frequency = 10,
            .cmpr_a = 0.f, // PWM_A duty cycle %
            .cmpr_b = 0.f, // PWM_B duty cycle %
            .duty_mode = MCPWM_DUTY_MODE_0,
            .counter_mode = MCPWM_UP_COUNTER
        };
        /* Init PWM device END */
        mcpwm_gpio_init(mcpwm_unit, MCPWM0A, pmotor->pin_PWM_A);
        if(pmotor->pin_PWM_B != MOT_PIN_NONE)
            mcpwm_gpio_init(mcpwm_unit, MCPWM0B, pmotor->pin_PWM_B);
        mcpwm_init(mcpwm_unit, MCPWM_TIMER_0, &mcpwm_config);
        mcpwm_set_duty_type(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0); // duty_type: ACTIVE HIGH
        if(pmotor->pin_PWM_B != MOT_PIN_NONE)
        {
            mcpwm_set_duty_type(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B, MCPWM_DUTY_MODE_0); // duty_type: ACTIVE HIGH
        }
        mcpwm_stop(mcpwm_unit, MCPWM_TIMER_0);
    }
    engage_brake(pmotor);
}

void engage_brake(Motor* pmotor)
{
    if(pmotor->pin_brake == MOT_PIN_NONE)
        return;
    SET_PIN(pmotor->pin_brake, MOTOR_OFF);
    pmotor->is_brake_released = false;
    pmotor->brake_engage_defer = 0;
}

void release_brake(Motor* pmotor)
{
    if(pmotor->pin_brake == MOT_PIN_NONE)
    {
        pmotor->is_brake_released = true;
        return;
    }
    SET_PIN(pmotor->pin_brake, MOTOR_ON);
    DELAY_MS(BRAKE_RELEASE_DELAY);
    pmotor->is_brake_released = true;
}

bool is_brake_released(Motor* pmotor)
{
    return (pmotor->is_brake_released);
}

void motor_set_speed(Motor* pmotor, uint8_t speed100)
{
    pmotor->target_speed = speed100;
    float duty_pct = (float)speed100;
    if (pmotor->mode_drive == MOT_RELAYS)
    {
        return;
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // single PWM mode, for speed control only
    {
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, duty_pct);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC)// two PWMs mode, for H-brige
    {
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, ((pmotor->status = MOT_CW)?(MCPWM_GEN_A):(MCPWM_GEN_B)), duty_pct);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
}

void run_CCW(Motor* pmotor)
{
    pmotor->status = MOT_CCW; 
    while(!is_brake_released(pmotor))
    {
        release_brake(pmotor);
    }
    if(pmotor->mode_drive == MOT_RELAYS)
    {
        SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
        SET_PIN(pmotor->pin_motor_N, MOTOR_ON);
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // if uses relays to control direction
    {
        motor_set_speed(pmotor,pmotor->target_speed);
        SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
        SET_PIN(pmotor->pin_motor_N, MOTOR_ON);
        
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC) // if use H-bridge to control direction and speed
    {
        float target_duty = (float)pmotor->target_speed;
        // mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B, target_duty);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
}

void run_CW(Motor* pmotor)
{
    pmotor->status = MOT_CW;
    while(!is_brake_released(pmotor))
    {
        release_brake(pmotor);
    }
    if(pmotor->mode_drive == MOT_RELAYS)
    {
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // if uses relays to control direction
    {
        motor_set_speed(pmotor,pmotor->target_speed);
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
        
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC) // if use H-bridge to control direction and speed
    {
        float target_duty = (float)pmotor->target_speed;
        // mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B);
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, target_duty);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
}

bool stop_motor(Motor* pmotor)
{
    if(pmotor->mode_drive == MOT_RELAYS)
    {
        SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // if uses relays to control direction
    {
        motor_set_speed(pmotor, 0); // cut-off the current before switching Relays
        mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        mcpwm_stop(pmotor->mcpwm_unit, MCPWM_TIMER_0);
        DELAY_MS(100); // wait a while before actuate relays to avoid spark
        SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC) // if use H-bridge to control direction and speed
    {
        motor_set_speed(pmotor, 0);
        mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B);
        mcpwm_stop(pmotor->mcpwm_unit, MCPWM_TIMER_0);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
    // inertia of the motor
    DELAY_MS(pmotor->brake_engage_defer * 2);
    pmotor->status = MOT_IDLE;
    return true;
}

void accelerate(Motor* pmotor){};