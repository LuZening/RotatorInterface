#include "motor.h"
#include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define tag "Motor"
// return:
// -1: x < 0
// 0: x == 0
// 1: x > 0
int8_t sign_zero_int32(int32_t x)
{
    return (int8_t)((x >> (31U - 7U)) | (x != 0));
}
#define SIGN_INT16(x) (x >> 15U)
#define SIGN_INT8(x) (x >> 7U)

static void DELAY_MS(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void init_motor(Motor* pmotor, mot_drive_mode_t mode_drive,
                int pin_brake, int pin_motor_N, int pin_motor_P, int pin_PWM_A, int pin_PWM_B, mcpwm_unit_t mcpwm_unit,
                int brake_engage_defer, uint32_t soft_start_duration_ms)
{
    pmotor->mode_drive = mode_drive;
    pmotor->pin_brake = pin_brake;
    pmotor->pin_motor_N = pin_motor_N;
    pmotor->pin_motor_P = pin_motor_P;
    pmotor->is_brake_released = false;
    pmotor->current_speed = 0;
    pmotor->target_speed = 0;
    pmotor->direction = 0;
    pmotor->target_direction = 0;
    pmotor->mcpwm_unit = mcpwm_unit;
    pmotor->pin_PWM_A = pin_PWM_A;
    pmotor->pin_PWM_B = pin_PWM_B;
    pmotor->brake_engage_defer = brake_engage_defer;
    pmotor->soft_start_duration_ms = soft_start_duration_ms;
    pmotor->soft_start_counter = 0;
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
        mcpwm_set_duty(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, 0);
        // mcpwm_set_duty_type(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0); // duty_type: ACTIVE HIGH
        mcpwm_set_signal_low(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        if(pmotor->pin_PWM_B != MOT_PIN_NONE)
        {
            mcpwm_set_duty(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B, 0);
            mcpwm_set_signal_low(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B);
            // mcpwm_set_duty_type(mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B, MCPWM_DUTY_MODE_0); // duty_type: ACTIVE HIGH
        }
        // mcpwm_stop(mcpwm_unit, MCPWM_TIMER_0);
    }
    engage_brake(pmotor);
    pmotor->motStatus = MOT_BRAKED;
}

void engage_brake(Motor* pmotor)
{
    if(pmotor->pin_brake == MOT_PIN_NONE)
        return;
    SET_PIN(pmotor->pin_brake, MOTOR_OFF);
    pmotor->is_brake_released = false;
    pmotor->motStatus = MOT_BRAKED;
}

void release_brake(Motor* pmotor)
{
    if(pmotor->pin_brake == MOT_PIN_NONE)
    {
        pmotor->is_brake_released = true;
        return;
    }
    SET_PIN(pmotor->pin_brake, MOTOR_ON);
    DELAY_MS(pmotor->brake_engage_defer);
    pmotor->is_brake_released = true;
    pmotor->motStatus = MOT_IDLE_WITH_BRAKE_RELEASED;
}

bool is_brake_released(Motor* pmotor)
{
    return (pmotor->is_brake_released);
}


void motor_set_target_speed_and_direction(Motor* pmotor, uint8_t speed100, int8_t direction)
{
    pmotor->target_speed = speed100;
    pmotor->target_direction = direction;
}

static void motor_set_speed(Motor* pmotor, uint8_t speed100)
{
    pmotor->current_speed = speed100;
    float duty_pct = 1.f * speed100;
    if (pmotor->mode_drive == MOT_RELAYS)
    {
        return;
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // single PWM mode, for speed control only
    {
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, duty_pct);
        mcpwm_set_duty_type(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
        ESP_LOGD(tag, "speed set to %.1f", duty_pct);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC)// two PWMs mode, for H-brige
    {
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, ((pmotor->direction == MOT_CW)?(MCPWM_GEN_A):(MCPWM_GEN_B)), duty_pct);
        mcpwm_set_duty_type(pmotor->mcpwm_unit, MCPWM_TIMER_0, ((pmotor->direction == MOT_CW)?(MCPWM_GEN_A):(MCPWM_GEN_B)), MCPWM_DUTY_MODE_0);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
}

void run_CCW(Motor* pmotor)
{
    int nInitialSpeed100;
    if(pmotor->soft_start_duration_ms >= 100)
    {
        nInitialSpeed100 = pmotor->target_speed / INITIAL_SPEED_DIVISION;
    }
    else
    {
        nInitialSpeed100 = pmotor->target_speed;
    }
    while(!is_brake_released(pmotor))
    {
        release_brake(pmotor);
    }
    pmotor->direction = MOT_CCW; 
    pmotor->motStatus = MOT_RUNNING;
    if(pmotor->mode_drive == MOT_RELAYS) // if uses relays to control direction, no speed control
    {
        // off means CCW
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
        // turn on the master power
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // if uses relays to control direction, uses PWM to control speed
    {
        // off means CCW
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
        // turn on the master power
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
        DELAY_MS(20); // to avoid spark
        motor_set_speed(pmotor, nInitialSpeed100);
        mcpwm_start(pmotor->mcpwm_unit, MCPWM_TIMER_0);
        ESP_LOGD(tag, "Run CCW init_speed=%d", nInitialSpeed100);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC) // if use H-bridge to control direction and speed
    {
        float target_duty = (float)nInitialSpeed100;
        // mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        pmotor->direction = -1;
        motor_set_speed(pmotor, nInitialSpeed100);
        mcpwm_start(pmotor->mcpwm_unit, MCPWM_TIMER_0);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_SPWM)
    {
        // TODO: implement SPWM motor driving mode
    }
}

void run_CW(Motor* pmotor)
{
    int nInitialSpeed100;
    if(pmotor->soft_start_duration_ms >= 100)
    {
        nInitialSpeed100 = pmotor->target_speed / INITIAL_SPEED_DIVISION;
    }
    else
    {
        nInitialSpeed100 = pmotor->target_speed;
    }
    while(!is_brake_released(pmotor))
    {
        release_brake(pmotor);
    }
    pmotor->direction = MOT_CW;
    pmotor->motStatus = MOT_RUNNING;
    if(pmotor->mode_drive == MOT_RELAYS)
    {
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
        SET_PIN(pmotor->pin_motor_N, MOTOR_ON);
    }
    else if(pmotor->mode_drive == MOT_PWM_RELAYS) // if uses relays to control direction
    {
        SET_PIN(pmotor->pin_motor_P, MOTOR_ON);
        SET_PIN(pmotor->pin_motor_N, MOTOR_ON);
        DELAY_MS(20); // to avoid spark
        mcpwm_start(pmotor->mcpwm_unit, MCPWM_TIMER_0);
        motor_set_speed(pmotor,nInitialSpeed100);
        ESP_LOGD(tag, "Run CW init_speed=%d", nInitialSpeed100);
    }
    else if(pmotor->mode_drive == MOT_FULL_BRIDGE_DC) // if use H-bridge to control direction and speed
    {
        float target_duty = (float)nInitialSpeed100;
        // mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_B);
        pmotor->direction = 1;
        mcpwm_set_duty(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A, target_duty);
        mcpwm_start(pmotor->mcpwm_unit, MCPWM_TIMER_0);
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
        // mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        motor_set_speed(pmotor, 0); // cut-off the current before switching Relays
        mcpwm_set_signal_low(pmotor->mcpwm_unit, MCPWM_TIMER_0, MCPWM_GEN_A);
        mcpwm_stop(pmotor->mcpwm_unit, MCPWM_TIMER_0);

        DELAY_MS(20); // wait a while before actuate relays to avoid spark
        SET_PIN(pmotor->pin_motor_P, MOTOR_OFF);
        SET_PIN(pmotor->pin_motor_N, MOTOR_OFF);
        ESP_LOGD(tag, "stopped");
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
    pmotor->current_speed = 0;
    pmotor->motStatus = MOT_IDLE_WITH_BRAKE_RELEASED;
    return true;
}

// call this handle routine frequently
int32_t motor_speed_update_handle( Motor* pmotor, int ms)
{
    int32_t nSignedCurrentSpeed = pmotor->current_speed * pmotor->direction; // CW: >0, CCW: <0
    int32_t nSignedTargetSpeed = pmotor->target_speed * pmotor->target_direction; // CW: >0, CCW: <0
    int32_t nSpeedDiff = nSignedCurrentSpeed - nSignedTargetSpeed;
    // calculate max value of speed change according to the setting of soft start duration
    // i.e. how much time is needed for the motor to reach 100% speed from still
    uint32_t nMaxSpeedChangeThisTime =  100 * ms / pmotor->soft_start_duration_ms;
    // calculate how much the speed can actually change
    int32_t nSpeedChange;
    if (nSpeedDiff < 0)
    {
        nSpeedChange = nMaxSpeedChangeThisTime;
        if(nSpeedChange + nSignedCurrentSpeed > nSignedTargetSpeed)
            nSpeedChange = nSignedTargetSpeed - nSignedCurrentSpeed;
    }
    else if(nSpeedDiff > 0)
    {
        nSpeedChange = -nMaxSpeedChangeThisTime;
        if(nSpeedChange + nSignedCurrentSpeed < nSignedTargetSpeed)
            nSpeedChange = nSignedTargetSpeed - nSignedCurrentSpeed;
    }
    else
    {
        nSpeedChange = 0;
    }
    int32_t nSignedSpeedNew = nSignedCurrentSpeed + nSpeedChange;
    if (nSpeedChange != 0)
    {

        if(abs(nSpeedChange) >= 10)
            ESP_LOGD(tag, "(%d ms) Current Speed %d", ms, nSignedSpeedNew);
        // when the sign of the new speed differs from that of the current speed
        if(sign_zero_int32(nSignedSpeedNew) != sign_zero_int32(nSignedCurrentSpeed))
        {
            ESP_LOGD(tag, "Change direction to %d", nSignedSpeedNew);
            if(nSignedCurrentSpeed != 0)
            {
                // stop the motor first (without engage the brake)
                motor_set_speed(pmotor, 0);
                // wait a while to avoid spark
                DELAY_MS(100);
            }
            if(nSignedSpeedNew != 0)
            {
                // change direction
                if(nSignedSpeedNew > 0)
                {
                    run_CW(pmotor);
                }
                else
                {
                    run_CCW(pmotor);
                }
            }
        }
        // set new speed
        uint8_t nSpeedNew = (nSignedSpeedNew >= 0)?(nSignedSpeedNew):(-nSignedSpeedNew);
        motor_set_speed(pmotor, nSpeedNew);
        // if target speed and new speed all reached 0, stop the motor
        if(nSpeedNew == 0 && nSignedTargetSpeed == 0 && nSignedCurrentSpeed != 0)
        {
            stop_motor(pmotor);
        }
    }
    return nSignedSpeedNew;
}