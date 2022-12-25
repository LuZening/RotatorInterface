#pragma once
#include "Lib595.h"
// TODO: status update
// TODO: REVERSE
#include "SPWM.h"
#include "driver/mcpwm.h"


// #define PIN_BREAK 0
// #define PIN_MOTOR_P 1
// #define PIN_MOTOR_N 2
#define MOT_PIN_NONE 0
#define MOTOR_ON 1
#define MOTOR_OFF 0
#define MOT_CCW -1
#define MOT_CW 1
#define MOT_DEADZONE 10
#define TIMEOUT_BREAK_RELEASE 200
#define INTERVAL_HANDLE_MOTOR 10
#define BRAKE_RELEASE_DELAY 300
#define BRAKE_ENGAGE_DELAY 300
#define INITIAL_SPEED_DIVISION 10


typedef enum
{
    MOT_RELAYS = 0, // use relays to control directon only
    MOT_PWM_RELAYS, // use relays to control direction, and use PWM to control speed
    /* MOT_FULL_BRIDGE_DC drive DC motor using FULL_BRIDGE
    pin_PWM_A: PWM
    pin_motor_N : polar
    */
    MOT_FULL_BRIDGE_DC, 
    /* MOT_FULL_BRIDGE_SPWM drive 1ph AC async motor using SPWM and FULL_BRIDGE with DC power supply
    pin_PWM_A: SPWM
    pin_motor_N: polar
    */
    MOT_FULL_BRIDGE_SPWM, 
    /* MOT_FULL_BRIDGE_SPWM_WITH_BRAKE drive 1ph AC async motor using SPWM and FULL_BRIDGE with DC power supply
    also drives the brake solenoid with SPWM wave
    pin_PWM_A: SPWM for motor
    pin_motor_N: polar for motor
    pin_PWM_B: SPWM for brake
    pin_motor_P: polar for brake
    */
    MOT_FULL_BRIDGE_SPWM_WITH_BRAKE,
} mot_drive_mode_t;

typedef enum
{
    MOT_BRAKED = 0,
    MOT_IDLE_WITH_BRAKE_RELEASED,
    MOT_RUNNING,
} MotStatus_t;

typedef struct
{
    mot_drive_mode_t mode_drive;
    SPWM_t spwm, spwmBrake;
    // set to MOT_PIN_NONE to ignore brake control
    uint8_t pin_brake; 
    /* pin_motor_P
    MOT_PWM_RELAYS: pin for master power
    MOT_FULL_BRIDGE_DC|MOT_FULL_BRIDGE_SPWM: None
    */
    uint8_t pin_motor_P; // pin for master power
    /* pin_motor_N
    MOT_PWM_RELAYS: pin for direction
    MOT_FULL_BRIDGE_DC|MOT_FULL_BRIDGE_SPWM: pin for H-Bridge polarity (determin high side conduction at left or right bridge)
    */
    uint8_t pin_motor_N; // pin for direction
    /* pin_PWM_A
    MOT_PWM_RELAYS: None
    MOT_FULL_BRIDGE_DC|MOT_FULL_BRIDGE_SPWM: pin for PWM
    */
    uint8_t pin_PWM_A;
    /* pin_PWM_B
    MOT_PWM_RELAYS: None
    MOT_FULL_BRIDGE_DC|MOT_FULL_BRIDGE_SPWM: None
    */
    uint8_t pin_PWM_B; // set to MOT_PIN_NONE if the motor's direction is not controlled by PWM
    mcpwm_unit_t mcpwm_unit;
    bool is_brake_released;
    int brake_engage_defer; // the interval between motor stop and break engagement, unit 
    MotStatus_t motStatus;
    int8_t direction; // 
    int8_t target_direction;
    uint8_t current_speed; // 0 ~ target_speed
    uint8_t target_speed; // 0 ~ 100
    uint32_t soft_start_counter;
    uint32_t soft_start_duration_ms;
} Motor;


void init_motor(Motor* pmotor, mot_drive_mode_t mode_drive,
                int pin_brake, int pin_motor_N, int pin_motor_P, int pin_PWM_A, int pin_PWM_B, mcpwm_unit_t mcpwm_unit,
                int brake_engage_defer, uint32_t soft_start_duration_ms);

// users control the motor by setting the target speed;
void motor_set_target_speed_and_direction(Motor* pmotor, uint8_t speed100, int8_t direction);
void engage_brake( Motor* pmotor);
void release_brake( Motor* pmotor);
bool is_brake_released( Motor* pmotor);
void run_CCW(Motor* pmotor);
void run_CW(Motor* pmotor);
bool stop_motor( Motor* pmotor);
int32_t motor_speed_update_handle( Motor* pmotor, int ms);
