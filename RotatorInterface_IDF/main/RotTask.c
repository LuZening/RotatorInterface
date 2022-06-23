#include "RotTask.h"

void init_task(RotTask_t* p)
{
    p->motnum = 1;
    p->type = ROT_TASK_NULL;
    
    p->is_executed = false;
    p->next = NULL;
}

void set_task(RotTask_t* p, int motnum, RotTaskType_t type, int n_to, int n_speed)
{
    p-> motnum = motnum;
    p->type = type;
    p->n_to = n_to;
    p->n_speed = n_speed;
    p->is_executed = false;
}