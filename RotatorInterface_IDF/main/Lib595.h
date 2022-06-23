#pragma once


#include "driver/gpio.h"

#define PIN_595_BEGIN 64U // Q1: 64 Q2: 65 Q3: 66
#define PIN_595(p) ((p) | PIN_595_BEGIN)
#define IS_PIN595(p) ((p) >= PIN_595_BEGIN)
#define SET_PIN(p, v) ((IS_PIN595(p))?(write_595(p595, (p) - PIN_595_BEGIN, v)):(gpio_set_level((p), (v))))
#define TO_595_PIN_NUMBER(n) ((n) | PIN_595_BEGIN)


typedef struct
{
    int pin_SCLK;
    int pin_RCLK;
    int pin_OE;
    int pin_DATA;
    unsigned char Q[8];
} IO595;

extern IO595 *p595;

void begin_595(IO595 *p595, int pin_SCLK, int pin_RCLK, int pin_OE, int pin_DATA);
void refresh_595( IO595 *p595);
void write_595(IO595 *p595, int pin, unsigned char v);