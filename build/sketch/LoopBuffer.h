#line 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\LoopBuffer.h"
#ifndef __LOOP_BUFFER_
#define __LOOP_BUFFER_
class LoopBuffer
{
private:
    int len;
    int head;
    int tail;
    int* buffer;
public:
    LoopBuffer(int);
    ~LoopBuffer(){delete buffer;}
    void append(int);
    int get_at(int);
    bool is_full();
    bool is_empty();
    int get_latest();
    int get_oldest();
    void clear();
};
#endif