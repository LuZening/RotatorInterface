#include "SPWM.h"
#include "fastmath.h"
#include "driver/timer.h"
#include "hal/timer_hal.h"
#include "driver/gpio.h"
#include "hal/gpio_hal.h"
uint32_t f;  // f: freq of target sinusodial waveform
uint32_t fm; // fm: pulse width update frequency
uint32_t fc; // fc: carrier frequency

#define TIMER_LL_GROUP TIMERG0
timer_group_t timer_group= TIMER_GROUP_0;
const uint32_t freqTimerHz = 1000000UL;

// retval: the length of waveform table
static void prepare_waveform_table(SPWM_t* p)
{
    uint32_t Ntry = p->fm / p->f / 2;
    if(Ntry > SPWM_MAX_TABLE_LEN)
    {
        Ntry = SPWM_MAX_TABLE_LEN;
        p->fm = p->f * Ntry * 2;
    }
    uint16_t nPeriodUS_m = 1000000UL / p->fm;
    p->N = (uint8_t)Ntry;
    for(uint8_t i = 0; i < p->N; ++i)
    {
        float phi = i * 3.1415926f / p->N;
        uint16_t us = (uint16_t)((p->alpha * sinf(phi) * (float)nPeriodUS_m) + 0.5f);
        p->waveform[i]  = us;
    }
}

static init_pwm_pins(SPWM_t* p)
{
    if(p->pinPWMA != SPWM_PIN_NULL)
    {
        gpio_set_level(p->pinPWMA, 0);
        gpio_set_direction(p->pinPWMA, GPIO_MODE_OUTPUT);
    }
    if(p->pinPolar != SPWM_PIN_NULL)
    {
        gpio_set_level(p->pinPolar, 0);
        gpio_set_direction(p->pinPolar, GPIO_MODE_OUTPUT);
    }
    if(p->pinPWMB != SPWM_PIN_NULL)
    {
        gpio_set_level(p->pinPWMB, 0);
        gpio_set_direction(p->pinPWMB, GPIO_MODE_OUTPUT);
    }
    if(p->pinPWMX != SPWM_PIN_NULL)
    {
        gpio_set_level(p->pinPWMX, 0);
        gpio_set_direction(p->pinPWMX, GPIO_MODE_OUTPUT);
    }
    if(p->pinPWMY != SPWM_PIN_NULL)
    {
        gpio_set_level(p->pinPWMY, 0);
        gpio_set_direction(p->pinPWMY, GPIO_MODE_OUTPUT);
    }
}

static inline set_pwm_pins(SPWM_t* p, bool isPolarChg)
{
    // set the PWM pins
    switch (p->mode)
    {
    case SPWM_SINGLE_PWM_PIN:
        if(isPolarChg)
        {
            if(p->nPolar >= 0)
            {
                gpio_ll_set_level(&GPIO, p->pinPolar, 1);
            }
            else
            {
                gpio_ll_set_level(&GPIO, p->pinPolar, 0);
            }
        }
        gpio_ll_set_level(&GPIO, p->pinPWMA, 1);
        break;
    // TODO:
    case SPWM_DUAL_PWM_PINS:
        break; 
    // TODO:
    case SPWM_QUAD_PWM_PINS:
        break;
    default:
        break;
    }
}

static inline clear_pwm_pins(SPWM_t* p)
{
    // set the PWM pins
    switch (p->mode)
    {
    case SPWM_SINGLE_PWM_PIN:
        gpio_ll_set_level(&GPIO, p->pinPWMA, 0);
        break;
    // TODO:
    case SPWM_DUAL_PWM_PINS:
        break; 
    // TODO:
    case SPWM_QUAD_PWM_PINS:
        break;
    default:
        break;
    }
}

static IRAM_ATTR bool timer_carrier_isr(void * args)
{
    SPWM_t* p = (SPWM_t*)args;
    bool isPolarChg = false;
    // stop the carrier timer
    set_pwm_pins(p, isPolarChg);
    timer_ll_set_counter_enable(&TIMER_LL_GROUP, TIMER_0, false);
    if(p->idx >= p->N)
    {
        p->idx = 0;
        p->nPolar = -p->nPolar;
        isPolarChg = true;
    }
    // start the duty time.
    // when the duty timer overflows, invalidate the PWM pins
    // timer_set_counter_value(timer_group, TIMER_1, 0);
    timer_ll_set_counter_value(&TIMER_LL_GROUP, TIMER_1, 0);
    // timer_set_alarm_value(timer_group, TIMER_1, p->waveform[p->idx++]);
    timer_ll_set_alarm_value(&TIMER_LL_GROUP, TIMER_1, p->waveform[p->idx++]);
    // timer_set_alarm(timer_group, TIMER_1, TIMER_ALARM_EN);
    timer_ll_set_alarm_enable(&TIMER_LL_GROUP, TIMER_1, true);
    // timer_start(timer_group, TIMER_1);
    timer_ll_set_counter_enable(&TIMER_LL_GROUP, TIMER_1, true);

    return true;
}

static IRAM_ATTR bool timer_duty_isr(void * args)
{
    SPWM_t* p = (SPWM_t*)args;
    // clear the PWM pins
    clear_pwm_pins(p);

    // stop the duty counter
    // timer_stop(timer_group, TIMER_1);
    timer_ll_set_counter_enable(&TIMER_LL_GROUP, TIMER_1, false);

    // restart the carrier timer for next pulse
    // timer_set_counter_value(timer_group, TIMER_0, 0);
    timer_ll_set_counter_value(&TIMER_LL_GROUP, TIMER_0, 0);
    // timer_set_alarm(timer_group,TIMER_0, TIMER_ALARM_EN);
    timer_ll_set_alarm_enable(&TIMER_LL_GROUP, TIMER_0, true);
    timer_ll_set_counter_enable(&TIMER_LL_GROUP, TIMER_0, true);
    // rewind the cursour of waveform table
    return true;
}

void SPWM_init(SPWM_t* p, SPWM_mode_t mode,
                uint8_t pinPWMA, uint8_t pinPolar, uint8_t pinPWMB, uint8_t pinPWMX, uint8_t pinPWMY,
                uint32_t f, uint32_t fm)
{
    p->mode = mode;
    p->N = 0;
    p->nPolar = 0;
    p->idx = 0;
    /* Init PWM device BEGIN */
    switch(mode)
    {
        case SPWM_SINGLE_PWM_PIN:
            p->pinPWMA = pinPWMA;
            p->pinPolar = pinPolar;
            p->pinPWMB = SPWM_PIN_NULL;
            p->pinPWMX = SPWM_PIN_NULL;
            p->pinPWMY = SPWM_PIN_NULL;
            
            break;
        case SPWM_DUAL_PWM_PINS:
            p->pinPWMA = pinPWMA;
            p->pinPWMB = pinPWMB;
            p->pinPolar = SPWM_PIN_NULL;
            p->pinPWMX = SPWM_PIN_NULL;
            p->pinPWMY = SPWM_PIN_NULL;
            break;
        case SPWM_QUAD_PWM_PINS:
            p->pinPWMA = pinPWMA;
            p->pinPWMB = pinPWMB;
            p->pinPolar = SPWM_PIN_NULL;
            p->pinPWMX = pinPWMX;
            p->pinPWMY = pinPWMY;
            break;
    }
    init_pwm_pins(p);
    timer_config_t timer_config= {
        .divider = (APB_CLK_FREQ / freqTimerHz), // tick at 1MHz, with precision to 1us
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_DIS,
    };
    // init carrier timer
    timer_init(timer_group, TIMER_0, &timer_config);
    timer_isr_callback_add(timer_group, TIMER_0, timer_carrier_isr, (void*)p, ESP_INTR_FLAG_IRAM);
    // init duty timer
    timer_init(timer_group, TIMER_1, &timer_config);
    timer_isr_callback_add(timer_group, TIMER_1, timer_duty_isr, (void*)p, ESP_INTR_FLAG_IRAM);
    SPWM_set_freq(p, f, fm);

}

void SPWM_set_freq(SPWM_t* p, uint32_t f, uint32_t fm)
{
    p->f = f;
    p->fm = fm;
    prepare_waveform_table(p);
    // set carrier timer's value
    // ftimer / fm
    p->nTimerFillValue_carrier = freqTimerHz / p->fm;
}


void SPWM_emit(SPWM_t* p)
{
    p->idx = 0;
    // initialize the Hbridge to output positive half first
    p->nPolar = 1;
    timer_set_counter_value(timer_group, TIMER_0, p->nTimerFillValue_carrier);
    timer_set_alarm_value(timer_group, TIMER_0, p->nTimerFillValue_carrier);
    timer_set_alarm(timer_group, TIMER_0, TIMER_ALARM_EN);
    timer_enable_intr(timer_group, TIMER_0);
    timer_set_counter_value(timer_group, TIMER_1, 0);
    timer_set_alarm_value(timer_group, TIMER_1, p->waveform[0]);
    timer_enable_intr(timer_group, TIMER_1);
    // start carrier
    timer_start(timer_group, TIMER_0);
}

void SPWM_stop(SPWM_t* p)
{
    timer_stop(timer_group, TIMER_0);
    p->nPolar = 0;
}


SPWM_t spwm;
SPWM_t* SPWM_unit_test_start()
{
    const uint8_t pinPWMA = 10, pinPolar = 11;
    const uint32_t f = 50, fm = 2000;
    SPWM_init(&spwm, SPWM_SINGLE_PWM_PIN, 
                pinPWMA, pinPolar, SPWM_PIN_NULL, SPWM_PIN_NULL, SPWM_PIN_NULL, 
                f, fm);
    SPWM_set_freq(&spwm, f, fm);
    SPWM_emit(&spwm);
    return &spwm;
}