#ifndef __LIB_595_H_
#define __LIB_595_H_


#define PIN_595_BEGIN 64
#define PIN_595(p) ((p) | PIN_595_BEGIN)
#define IS_PIN595(p) ((p) >= PIN_595_BEGIN)
#define SET_PIN(p, v) ((IS_PIN595(p))?(write_595(p595, p - PIN_595_BEGIN, v)):(digitalWrite(p, v)))

struct IO595
{
    int pin_SCLK;
    int pin_RCLK;
    int pin_OE;
    int pin_DATA;
    unsigned char Q[8];
} _io595;

struct IO595 *p595 = &_io595;

void begin_595(struct IO595 *p595, int pin_SCLK, int pin_RCLK, int pin_OE, int pin_DATA);
void refresh_595(struct IO595 *p595);
void write_595(struct IO595 *p595, int pin, unsigned char v);

void begin_595(struct IO595 *p595, int pin_SCLK, int pin_RCLK, int pin_OE, int pin_DATA)
{
    memset(p595->Q, 1, 8);
    p595->pin_SCLK = pin_SCLK;
    p595->pin_RCLK = pin_RCLK;
    p595->pin_OE = pin_OE;
    p595->pin_DATA = pin_DATA;
    pinMode(pin_SCLK, OUTPUT);

    pinMode(pin_RCLK, OUTPUT);
    pinMode(pin_OE, OUTPUT);
    pinMode(pin_DATA, OUTPUT);
    digitalWrite(pin_OE, HIGH);
    refresh_595(p595);
    digitalWrite(pin_OE, LOW);
};

void write_595(struct IO595 *p595, int pin, unsigned char v)
{
    p595->Q[pin] = v;
    refresh_595(p595);
}

void refresh_595(struct IO595 *p595)
{
    int i;
    digitalWrite(p595->pin_RCLK, LOW);
    for(i=7; i>=0; --i)
    {
        digitalWrite(p595->pin_SCLK, LOW);
        delayMicroseconds(1);
        digitalWrite(p595->pin_DATA, p595->Q[i]);
        delayMicroseconds(1);
        digitalWrite(p595->pin_SCLK, HIGH);
        delayMicroseconds(1);
    }
    digitalWrite(p595->pin_RCLK, HIGH);
    delayMicroseconds(1);
}
#endif