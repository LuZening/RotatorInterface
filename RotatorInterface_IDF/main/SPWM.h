#pragma once

#define SPWM_MAX_TABLE_LEN 64
#define SPWM_PIN_NULL 0

typedef enum
{
    SPWM_SINGLE_PWM_PIN = 0,
    SPWM_DUAL_PWM_PINS,
    SPWM_QUAD_PWM_PINS,
} SPWM_mode_t;

typedef struct
{
    SPWM_mode_t mode;
    /* Pin Assignments BEGIN */
    /* pinPWMA 
    SPWM_SIGNLE_PWM_PIN: single ended PWM waveform
    SPWM_DUAL_PWM_PINS: PWM waveform for left bridge
    SPWM_QUAD_PWM_PINS: PWM waveform for upper-left MOSFET
     */
    uint8_t pinPWMA;
    /* pinDir
    SPWM_SIGNLE_PWM_PIN: polarity pin
    SPWM_DUAL_PWM_PINS: None
    SPWM_QUAD_PWM_PINS: None
     */
    uint8_t pinPolar; 
    /* pinPWMB
    SPWM_DUAL_PWM_PIN:  PWM waveform for right bridge
    SPWM_QUAD_PWM_PIN:  PWM waveform for lower-left MOSFET
    */
    uint8_t pinPWMB;
    /* pinPWMX pinPWMY
    SPWM_DUAL_PWM_PIN:  None
    SPWM_QUAD_PWM_PIN:  PWM waveform for upper-right, lower-right MOSFETs
    */
    uint8_t pinPWMX, pinPWMY;
    /* Pin Assignments END */


    /* Waveform parameters */
    uint32_t f, fm;
    uint8_t N;
    float alpha; // 0-1, modulation depth
    uint16_t waveform[SPWM_MAX_TABLE_LEN];
    uint32_t nTimerFillValue_carrier;

    /* working conditions BEGIN */
    int8_t nPolar; // 1: positive, 0: stop, -1: negative
    uint8_t idx;
    uint8_t curDuty; // [0, 255] MAX
    /* working conditions END */

} SPWM_t;

void SPWM_init(SPWM_t* p, SPWM_mode_t mode, 
                uint8_t pinPWMA, uint8_t pinPolar, uint8_t pinPWMB, uint8_t pinPWMX, uint8_t pinPWMY,
                uint32_t f, uint32_t fm);

void SPWM_set_freq(SPWM_t* p, uint32_t f, uint32_t fm);

void SPWM_emit(SPWM_t* p);

void SPWM_stop(SPWM_t* p);


extern SPWM_t spwm;
SPWM_t* SPWM_unit_test_start();