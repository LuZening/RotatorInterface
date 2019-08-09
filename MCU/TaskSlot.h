#ifndef __TASK_SLOT_
#define __TASK_SLOT_
#include <Arduino.h>
#include "RotSensor.h"
#define MANUAL 1
#define TARGET 2
#define CALIBRATE 3
#define NULL_TASK 0
#define INFINITE_TIME 0xffffff
#define MANUAL_INTERVAL 10000          // ms
#define MAX_SPEED 255
struct Task
{
    int type; // NULL OR Manual OR Target
    int n_to; // -ms~+ms OR -EXT~360+EXT
    int n_speed; // 0-255
    bool is_executed;
    struct Task* next;
};

void init_task(Task* p);
void set_task(Task* p, int type, int n_to, int n_speed);

#endif