#include "TaskSlot.h"
void init_task(Task* p)
{
    p->type = NULL_TASK;
    p->is_executed = false;
    p->next = NULL;
}

void set_task(Task* p, int type, int n_to, int n_speed)
{
    p->type = type;
    p->n_to = n_to;
    p->n_speed = n_speed;
    p->is_executed = false;
}