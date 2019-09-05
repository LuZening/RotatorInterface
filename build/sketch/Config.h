#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <Arduino.h>
#include "AT24C.h"

#define _ID "VORTEX2"
#define PAGE_CONFIG 1
#define PAGE_ACTIVE_BEGIN 16
typedef unsigned char byte;


struct Config
{
    // identity
    char s_id[8];
    // rotator
    int pot_type; // type of potentiometer wiring 2-Terminals or 3-Terminals
    bool allow_multi_rounds;
    unsigned int R_0; // pre-dividing resistor 500Ohm
    unsigned int R_c; // zero-level elevation resistor 51Ohm
    unsigned int R_max; // potentiometer resistance range
    unsigned int R_min; 
    unsigned int ADC_min;
    unsigned int ADC_max;
    unsigned int ADC_zero;
    unsigned int ADC_sensor_gap;
    bool inverse_ADC;
    int deg_limit_CW;
    int deg_limit_CCW;
    int break_engage_defer;
    bool is_ADC_calibrated;
    // network access
    char s_name[32];
    char s_ssid[32];
    char s_password[32];
    bool is_WiFi_set;
};

struct ActiveParams
{
    // EEPROM write CYCLES
    unsigned int n_cycles;
    int ADC_reading;
    int n_rounds;
    int n_degree;
};
union ConfigWriteBlock
{
    struct Config body;
    byte bytes[sizeof(struct Config)];
};

union ActiveWriteBlock
{
    struct ActiveParams body;
    byte bytes[sizeof(struct ActiveParams)];
};

extern byte page_active;
extern union ConfigWriteBlock *p_cfg;
extern union ActiveWriteBlock *p_actprm;

void load_config(union ConfigWriteBlock* p_cfg);
void load_active_params(union ActiveWriteBlock* p_actprm);
void save_config(union ConfigWriteBlock* p_cfg);
void save_active_params(union ActiveWriteBlock* p_actprm);
#endif
