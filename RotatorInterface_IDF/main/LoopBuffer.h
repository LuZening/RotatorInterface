#pragma once
#include "stdbool.h"
#include "stdint.h"
typedef struct 
{
    int len;
    int head;
    int tail;
    /* data */
    int* buffer;
} LoopBuffer;

void LoopBuffer_init(LoopBuffer *p, int n);
void LoopBuffer_append(LoopBuffer* p, int v);
int LoopBuffer_get_at(LoopBuffer* p, int idx);
bool LoopBuffer_is_full(LoopBuffer* p);
bool LoopBuffer_is_empty(LoopBuffer* p);
int LoopBuffer_get_latest(LoopBuffer* p);
int LoopBuffer_get_oldest(LoopBuffer *p);
void LoopBuffer_clear(LoopBuffer *p);