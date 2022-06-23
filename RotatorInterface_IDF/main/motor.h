#pragma once

// #define PIN_BREAK 0
// #define PIN_MOTOR_P 1
// #define PIN_MOTOR_N 2
#define MOT_PIN_NONE 0
#define MOTOR_ON 1
#define MOTOR_OFF 0
#define MOT_CCW -1
#define MOT_CW 1
#define MOT_IDLE 0 
#define MOT_DEADZONE 10
#define TIMEOUT_BREAK_RELEASE 200
#define INTERVAL_HANDLE_MOTOR 10
#define BRAKE_RELEASE_DELAY 300
#define BRAKE_ENGAGE_DELAY 300

#include "Lib595.h"
// TODO: status update
// TODO: REVERSE
#include "driver/mcpwm.h"

typedef enum
{
    MOT_RELAYS = 0, // use relays to control directon only
    MOT_PWM_RELAYS, // use relays to control direction, and use PWM to control speed
    MOT_FULL_BRIDGE_DC, // drive DC motor using FULL_BRIDGE
    MOT_FULL_BRIDGE_SPWM //drive 1ph AC async motor using SPWM and FULL_BRIDGE with DC power supply
} mot_drive_mode_t;


typedef struct
{
    mot_drive_mode_t mode_drive;
    int pin_brake; // set to MOT_PIN_NONE to ignore brake control
    int pin_motor_P; // pin to control positive coil of the motor
    int pin_motor_N; // pin to control negative coil of the motor, set to MOT_PIN_NONE if the motor only have 2 electrods
    int pin_PWM_A;
    int pin_PWM_B; // set to MOT_PIN_NONE if the motor's direction is not controlled by PWM
    mcpwm_unit_t mcpwm_unit;
    bool is_brake_released;
    int brake_engage_defer; // the interval between motor stop and break engagement, unit 
    int status;
    uint8_t target_speed; // 0 ~ 100
} Motor;

void init_motor(Motor* pmotor, mot_drive_mode_t mode_drive, int pin_brake, int pin_motor_N, int pin_motor_P, int pin_PWM_A, int pin_PWM_B, mcpwm_unit_t mcpwm_unit);
void engage_brake( Motor* pmotor);
void release_brake( Motor* pmotor);
bool is_brake_released( Motor* pmotor);
void motor_set_speed(Motor* pmotor, uint8_t speed100);
void run_CCW(Motor* pmotor);
void run_CW(Motor* pmotor);
bool stop_motor( Motor* pmotor);
void accelerate( Motor* pmotor);
