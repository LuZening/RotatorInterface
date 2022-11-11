#pragma once

#include "motor.h"

#ifdef FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#define USE_MUTEX_ON_CFG 1
#endif
// by default use EEPROM 24C64 with 8KBytes capacity

#define FS_BASE_PATH_CONFIG "/spiflash"
#define FS_PATH_CONFIG_FILE "/spiflash/cfg.bin"

#define VORTEX_VALID_STRING "VORTEX3"
#define CONFIG_BYTESTRING_LEN 32
#define ADDR_CONFIG_BEGIN 0
#define ADDR_ACTIVE_STATIC
// #define ADDR_ACTIVE_DYNAMIC
#define ADDR_ACTIVE_BEGIN 1024
typedef unsigned char byte;

#define N_POT_INPUTS 2
#define N_MOT_OUTPUTS 2


typedef enum __attribute__((__packed__))
{
    POT_3_WIRES,
    POT_2_WIRES
} PotType_t;

/* do not use float parameters */
// type of variables: uint8_t, int, char
struct Config
{
    // identity
    char sValid[CONFIG_BYTESTRING_LEN];
    char s_name[CONFIG_BYTESTRING_LEN];
    // rotator
    uint8_t N_POTs; // number of available potentiometers pluged in
    uint8_t pot_type[N_POT_INPUTS]; // type of potentiometer wiring 2-Terminals or 3-Terminals
    uint8_t allow_multi_rounds[N_POT_INPUTS];
    int R_0[N_POT_INPUTS]; // pre-dividing resistor, by default 1KOhm
    int R_c[N_POT_INPUTS]; // zero-level elevation resistor 51Ohm
    int R_max[N_POT_INPUTS]; // potentiometer resistance range
    int R_min[N_POT_INPUTS]; 
    int ADC_min[N_POT_INPUTS];
    int deg_min[N_POT_INPUTS];
    int ADC_max[N_POT_INPUTS];
    int deg_max[N_POT_INPUTS];
    int ADC_zero[N_POT_INPUTS];
    int ADC_sensor_gap[N_POT_INPUTS];
    uint8_t inverse_ADC[N_POT_INPUTS];
    int deg_limit_CW[N_POT_INPUTS];
    int deg_limit_CCW[N_POT_INPUTS];
    int brake_engage_defer[N_MOT_OUTPUTS];
    int soft_start_duration[N_MOT_OUTPUTS];
    uint8_t is_ADC_calibrated[N_POT_INPUTS];
    uint8_t N_MOTs; // number of motors
    uint8_t motDriveModes[N_MOT_OUTPUTS];
    uint8_t motSpeeds[N_MOT_OUTPUTS]; // 0 ~ 100
    // network access
    uint8_t use_WiFi;
    char WiFi_SSID[CONFIG_BYTESTRING_LEN];
    char WiFi_password[CONFIG_BYTESTRING_LEN];
    uint8_t use_Ethernet;
    uint8_t use_RS485;
}; // size of config is around 192Bytes

typedef struct Config Config;

extern struct Config cfg;

struct ActiveParams
{
    // EEPROM write CYCLES
    unsigned int n_cycles;
    int ADC_reading;
    int n_rounds;
    int n_degree;
}; // size of ActiveParams is about 16Bytes
typedef struct ActiveParams ActiveParams;

extern ActiveParams actprm;

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

/* Config name->pointer mapper  */
typedef enum
{
    CONFIG_VAR_U8 = 0,
    CONFIG_VAR_I32,
    CONFIG_VAR_BYTESTRING
} config_var_type_t;
typedef struct
{
    const char* name;
    void* pV;
    config_var_type_t typ;
} config_var_map_t;

extern const config_var_map_t configNameMapper[];

#ifdef USE_MUTEX_ON_CFG
extern SemaphoreHandle_t mtxConfig;
#endif

void load_config(union ConfigWriteBlock* p_cfg);
// if config read from EEPROM is not 
void init_config(struct Config *p);


void load_active_params(union ActiveWriteBlock* p_actprm);
void save_config(union ConfigWriteBlock* p_cfg);
void save_active_params(union ActiveWriteBlock* p_actprm);
bool config_check_valid(struct Config* p);

// set the value of the config variable by name
bool set_config_variable_by_name(const char* name, const void* pV);
// get the value of the config variable by name
// @name the name of the config variable
// @buf the memory to store the value of the variable. If the variable's value is a string, the size of the buf must >= 32 
bool get_config_variable_by_name(const char* name, void* buf);
bool get_if_config_modified();
// get the pointer to the config variable by name
// return NULL if not found
config_var_map_t* get_config_variable_mapper_item_by_name(const char* name);

// Sync working objects, such as RotSensor and etc, with config variables
// call after config variables are updated
void push_config_to_volatile_variables(struct Config* p);


int get_config_string(char *buf, int lenbuf);