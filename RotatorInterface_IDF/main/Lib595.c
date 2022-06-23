#include "Lib595.h"
#include "string.h"

IO595 _io595;
IO595 *p595 = &_io595;

void begin_595(IO595 *p595, int pin_SCLK, int pin_RCLK, int pin_OE, int pin_DATA)
{
    memset(p595->Q, 1, 8);
    p595->pin_SCLK = pin_SCLK;
    p595->pin_RCLK = pin_RCLK;
    p595->pin_OE = pin_OE;
    p595->pin_DATA = pin_DATA;
    //
    // set GPIO pin_SCLK
    gpio_set_direction(pin_SCLK, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(pin_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_level(pin_SCLK, 1);

    // set GPIO pin_RCLK
    gpio_set_direction(pin_RCLK, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(pin_RCLK, GPIO_PULLUP_ONLY);
    gpio_set_level(pin_RCLK, 1);

    // set GPIO pin_DATA
    gpio_set_direction(pin_DATA, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(pin_DATA, GPIO_PULLUP_ONLY);
    gpio_set_level(pin_DATA, 1);

    // set GPIO pin_OE
    gpio_set_level(pin_DATA, 1);
    gpio_set_direction(pin_DATA, GPIO_MODE_OUTPUT_OD);
    gpio_set_pull_mode(pin_DATA, GPIO_PULLUP_ONLY);
   

    refresh_595(p595);

};

void write_595(IO595 *p595, int pin, unsigned char v)
{
    p595->Q[pin] = v;
    refresh_595(p595);
}

static void delay_us(unsigned int i)
{
    // by default 100MHz
    volatile unsigned int n = i * 20;
    while(n--);
}

void refresh_595( IO595 *p595)
{
    int i;
    gpio_set_level(p595->pin_RCLK, 0);
    for (i = 7; i >= 0; --i)
    {
        gpio_set_level(p595->pin_SCLK, 0);
        // delayMicroseconds(1);
        delay_us(1);
        gpio_set_level(p595->pin_DATA, p595->Q[i]);
        // delayMicroseconds(1);
        delay_us(1);
        gpio_set_level(p595->pin_SCLK, 1);
        // delayMicroseconds(1);
        delay_us(1);
    }
    gpio_set_level(p595->pin_RCLK, 1);
    // delayMicroseconds(1);
}