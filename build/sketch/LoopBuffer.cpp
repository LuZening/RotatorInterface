#include "LoopBuffer.h"
#include "Arduino.h"
LoopBuffer::LoopBuffer(int n)
{
    len = n;
    head = -1;
    tail = 0;
    buffer = (int*)malloc(sizeof(int) * n);
}

void LoopBuffer::append(int data)
{
    buffer[tail] = data;
    if(tail == head)
        head = (head+1) % len;
    if(head == -1) head = 0;
    tail = (tail+1) % len;
}
int LoopBuffer::get_at(int idx)
{
    return buffer[(head + idx) % len];
}
bool LoopBuffer::is_full()
{
    return (head == tail);
}
bool LoopBuffer::is_empty()
{
    return (head == -1);
}
int LoopBuffer::get_latest()
{
    if(is_empty())
    {
        return -255;
    }
    else
    {
        return buffer[((tail - 1 < 0)?(len-1):(tail-1))];
    }
}
int LoopBuffer::get_oldest()
{
    if(is_empty())
    {
        return -255;
    }
    else
    {
        return buffer[head];
    }
}

void LoopBuffer::clear()
{
    head = -1;
    tail = 0;
}

