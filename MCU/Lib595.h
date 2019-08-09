#ifndef __LIB_595_H_
#define __LIB_595_H_

#include<Arduino.h>

#define PIN_595_BEGIN 64
#define PIN_595(p) ((p) | PIN_595_BEGIN)
#define IS_PIN595(p) ((p) >= PIN_595_BEGIN)
#define SET_PIN(p, v) ((IS_PIN595(p))?(write_595(p595, (p) - PIN_595_BEGIN, v)):(digitalWrite((p), (v))))

struct IO595
{
    int pin_SCLK;
    int pin_RCLK;
    int pin_OE;
    int pin_DATA;
    unsigned char Q[8];
};

extern struct IO595 *p595;

void begin_595(struct IO595 *p595, int pin_SCLK, int pin_RCLK, int pin_OE, int pin_DATA);
void refresh_595(struct IO595 *p595);
void write_595(struct IO595 *p595, int pin, unsigned char v);
#endif