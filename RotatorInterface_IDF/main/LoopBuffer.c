#include "LoopBuffer.h"
#include "freertos/FreeRTOS.h"

void LoopBuffer_init(LoopBuffer *p, int n)
{
    p->len = n;
    p->head = -1;
    p->tail = 0;
    p->buffer = (int*)pvPortMalloc(sizeof(int) * n);
}

void LoopBuffer_append(LoopBuffer *p, int data)
{
    p->buffer[p->tail] = data;
    if(p->tail == p->head)
        p->head = (p->head+1) % p->len;
    if(p->head == -1) p->head = 0;
    p->tail = (p->tail+1) % p->len;
}
int LoopBuffer_get_at(LoopBuffer *p, int idx)
{
    return p->buffer[(p->head + idx) % p->len];
}

bool LoopBuffer_is_full(LoopBuffer *p)
{
    return (p->head == p->tail);
}
bool LoopBuffer_is_empty(LoopBuffer *p)
{
    return (p->head == -1);
}
int LoopBuffer_get_latest(LoopBuffer *p)
{
    if(LoopBuffer_is_empty(p))
    {
        return -255;
    }
    else
    {
        return p->buffer[((p->tail - 1 < 0)?(p->len-1):(p->tail-1))];
    }
}
int LoopBuffer_get_oldest(LoopBuffer *p)
{
    if(LoopBuffer_is_empty(p))
    {
        return -255;
    }
    else
    {
        return p->buffer[p->head];
    }
}

void LoopBuffer_clear(LoopBuffer *p)
{
    p->head = -1;
    p->tail = 0;
}

