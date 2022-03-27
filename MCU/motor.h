#ifndef __MOTOR_H_
#define __MOTOR_H_

#include <Arduino.h>

// #define PIN_BREAK 0
// #define PIN_MOTOR_P 1
// #define PIN_MOTOR_N 2
#define MOTOR_ON LOW
#define MOTOR_OFF HIGH
#define MOT_CCW -1
#define MOT_CW 1
#define MOT_IDLE 0 
#define MOT_DEADZONE 10
#define TIMEOUT_BREAK_RELEASE 200
#define INTERVAL_HANDLE_MOTOR 10
#define BREAK_RELEASE_DELAY 300
#define BREAK_ENGAGE_DELAY 300

#include "Lib595.h"
// TODO: status update
// TODO: REVERSE
struct Motor
{
    int pin_break;
    int pin_motor_P;
    int pin_motor_N;
    bool is_break_released;
    int break_engage_defer; // the interval between motor stop and break engagement
    int status;
};

void init_motor(struct Motor* pmotor, int pin_break, int pin_motor_N, int pin_motor_P);
void engage_break(struct Motor* pmotor);
void release_break(struct Motor* pmotor);
bool is_break_released(struct Motor* pmotor);
void run_CCW(struct Motor* pmotor);
void run_CW(struct Motor* pmotor);
bool stop_motor(struct Motor* pmotor);
void accelerate(struct Motor* pmotor);

#endif