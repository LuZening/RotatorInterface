#pragma once

#include "RotSensor.h"
#define MANUAL 1
#define TARGET 2
#define CALIBRATE 3
#define NULL_TASK 0
#define INFINITE_TIME 0x7ffffff
#define MANUAL_INTERVAL 75000          // ms
#define MAX_SPEED 255

typedef enum 
{
    ROT_TASK_NULL = 0,
    ROT_TASK_MANUAL,
    ROT_TASK_TARGET
} RotTaskType_t;
typedef struct 
{
    int motnum;
    RotTaskType_t type; // NULL OR Manual OR Target
    int n_to; // -ms~+ms OR -EXT~360+EXT
    int n_speed; // 0-255
    bool is_executed;
    struct Task* next;
} RotTask_t;

void init_task(RotTask_t* p);
void set_task(RotTask_t* p, int motnum, RotTaskType_t type, int n_to, int n_speed);

