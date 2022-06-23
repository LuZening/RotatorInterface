#include "Config.h"


#include "esp_log.h"
#include <string.h>
#include "main.h"
#include "string.h"
#include "AT24C.h"

Config cfg;
ActiveParams actprm;

byte page_active;


// use '.' to indicate the positon in an array
// for '.' is well supported in HTML URL encoding
// either '()' or '[]' will be converted in HTML URL
const config_var_map_t configNameMapper[] = {
    {"N_POTs", &(cfg.N_POTs), CONFIG_VAR_U8},
    /* pot config array */
    {"pot_type.1", &(cfg.pot_type[0]), CONFIG_VAR_U8},
    {"allow_multi_rounds.1", &(cfg.allow_multi_rounds[0]), CONFIG_VAR_U8},
    {"R_0.1", &(cfg.R_0[0]), CONFIG_VAR_I32},
    {"R_c.1", &(cfg.R_c[0]), CONFIG_VAR_I32},
    {"R_max.1", &(cfg.R_max[0]), CONFIG_VAR_I32},
    {"R_min.1", &(cfg.R_min[0]), CONFIG_VAR_I32},
    {"ADC_min.1", &(cfg.ADC_min[0]), CONFIG_VAR_I32},
    {"deg_min.1", &(cfg.deg_min[0]), CONFIG_VAR_I32},
    {"ADC_max.1", &(cfg.ADC_max[0]), CONFIG_VAR_I32},
    {"deg_max.1", &(cfg.deg_max[0]), CONFIG_VAR_I32},
    {"ADC_zero.1", &(cfg.ADC_zero[0]), CONFIG_VAR_I32},
    {"ADC_sensor_gap.1", &(cfg.ADC_sensor_gap[0]), CONFIG_VAR_I32},
    {"inverse_ADC.1", &(cfg.inverse_ADC[0]), CONFIG_VAR_U8},
    {"deg_limit_CW.1", &(cfg.deg_limit_CW[0]), CONFIG_VAR_I32},
    {"deg_limit_CCW.1", &(cfg.deg_limit_CCW[0]), CONFIG_VAR_I32},
    {"brake_engage_defer.1", &(cfg.brake_engage_defer[0]), CONFIG_VAR_I32},
    {"is_ADC_calibrated.1", &(cfg.is_ADC_calibrated[0]), CONFIG_VAR_U8},
    {"pot_type.2", &(cfg.pot_type[1]), CONFIG_VAR_U8},
    {"allow_multi_rounds.2", &(cfg.allow_multi_rounds[1]), CONFIG_VAR_U8},
    {"R_0.2", &(cfg.R_0[1]), CONFIG_VAR_I32},
    {"R_c.2", &(cfg.R_c[1]), CONFIG_VAR_I32},
    {"R_max.2", &(cfg.R_max[1]), CONFIG_VAR_I32},
    {"R_min.2", &(cfg.R_min[1]), CONFIG_VAR_I32},
    {"ADC_min.2", &(cfg.ADC_min[1]), CONFIG_VAR_I32},
    {"deg_min.2", &(cfg.deg_min[1]), CONFIG_VAR_I32},
    {"ADC_max.2", &(cfg.ADC_max[1]), CONFIG_VAR_I32},
    {"deg_max.2", &(cfg.deg_max[1]), CONFIG_VAR_I32},
    {"ADC_zero.2", &(cfg.ADC_zero[1]), CONFIG_VAR_I32},
    {"ADC_sensor_gap.2", &(cfg.ADC_sensor_gap[1]), CONFIG_VAR_I32},
    {"inverse_ADC.2", &(cfg.inverse_ADC[1]), CONFIG_VAR_U8},
    {"deg_limit_CW.2", &(cfg.deg_limit_CW[1]), CONFIG_VAR_I32},
    {"deg_limit_CCW.2", &(cfg.deg_limit_CCW[1]), CONFIG_VAR_I32},
    {"brake_engage_defer.2", &(cfg.brake_engage_defer[1]), CONFIG_VAR_I32},
    {"is_ADC_calibrated.2", &(cfg.is_ADC_calibrated[1]), CONFIG_VAR_U8},
    /* Motor config array */
    {"N_MOTs", &(cfg.N_MOTs), CONFIG_VAR_U8},
    {"motDriveModes.1", &(cfg.motDriveModes[0]), CONFIG_VAR_U8},
    {"motDriveModes.2", &(cfg.motDriveModes[1]), CONFIG_VAR_U8},
    /* Others */
    {"use_WiFi", &(cfg.use_WiFi), CONFIG_VAR_U8},
    {"s_name", (cfg.s_name), CONFIG_VAR_BYTESTRING},
};


void init_config(struct Config *p)
{
    int i;
    // set valid string
    strncpy(p->sValid, VORTEX_VALID_STRING, 8);
    // set POT settings
    p->N_POTs = 1;
    p->N_MOTs = 1;
    for(i = 0; i < p->N_MOTs; ++i)
    {
        p->motDriveModes[i] = MOT_PWM_RELAYS;
    }
    for(i = 0; i < p->N_POTs; ++i)
    {
        p->pot_type[i] = POT_3_WIRES;
        p->allow_multi_rounds[i] = false;
        p->R_0[i] = 1000;
        p->R_c[i] = 51;
        p->R_max[i] = 1000;
        p->R_min[i] = 1;
        p->ADC_min[i] = 0;
        p->deg_min[i] = 0;
        p->ADC_max[i] = 1023;
        p->deg_max[i] = 360;
        p->ADC_zero[i] = 512;
        p->ADC_sensor_gap[i] = 1023;
        p->inverse_ADC[i] = false;
        p->deg_limit_CCW[i] = 0;
        p->deg_limit_CW[i] = 359;
        p->brake_engage_defer[i] = 1000000; // us
        p->is_ADC_calibrated[i] = false;
    }
    // set network settings
    p->use_WiFi = true;
    strncpy(p->s_name, VORTEX_VALID_STRING, sizeof(p->s_name)-1);
}


void push_config_to_volatile_variables(struct Config* p)
{
    int i;
    for(i = 0; i < p->N_MOTs; ++i)
    {
        arrMotors[i].mode_drive = p->motDriveModes[i];
    }
    for(i = 0; i < p->N_POTs; ++i)
    {
        RotSensor* pRot = &(arrRotSensors[i]);
        pRot->pot_type = p->pot_type[i];
        RotSensor_init(pRot, p->pot_type[i], p->allow_multi_rounds[i], p->brake_engage_defer[i]);
        // pRot->R_0 = p->R_0[i];
        // pRot->R_c =p->R_c[i];
        // pRot->R_max = p->R_max[i];
        // pRot->R_min = p->R_min[i];
        RotSensor_set_resistors(pRot, p->R_max[i],p->R_min[i], p->R_0[i], p->R_c[i]);
        // pRot->ADC_min = p->ADC_min[i];
        // pRot->deg_min = p->deg_min[i];
        // pRot->ADC_max = p->ADC_max[i];
        // pRot->deg_max = p->deg_max[i];
        // pRot->ADC_zero = p->ADC_zero[i];
        RotSensor_set_ADC_range(pRot, p->ADC_min[i], p->ADC_max[i], p->ADC_zero[i], p->deg_min[i], p->deg_max[i]);
        pRot->ADC_sensor_gap = p->ADC_sensor_gap[i];
        pRot->inverse_ADC = p->inverse_ADC[i];
        pRot->deg_limit_F = p->deg_limit_CW[i];
        pRot->deg_limit_B = p->deg_limit_CCW[i];
        pRot->is_ADC_calibrated = p->is_ADC_calibrated[i];
    }
}

void load_config(union ConfigWriteBlock* p_cfg)
{
    EEPROM_ReadRange(&eeprom, ADDR_CONFIG_BEGIN, p_cfg->bytes, sizeof(Config));
    // read_array_AT24C(pEEPROM, PAGE2ADDR(pEEPROM, PAGE_CONFIG), p_cfg->bytes, sizeof(ConfigWriteBlock));
    // ESP_LOGD("CONFIG", "ssid=%s, passwd=%s", p_cfg->body.s_ssid, p_cfg->body.s_password);
}

void load_active_params(union ActiveWriteBlock* p_actprm)
{
    uint16_t addrActiveBegin;
    // 使用静态位置
    uint16_t addr_active_begin = ADDR_ACTIVE_BEGIN;
    // 使用动态位置
    // page_active = read_AT24C(pEEPROM, 0);
    // Serial.print("EEPROM PAGE ACTIVE:");
    // Serial.println(page_active);
    // 如果动态读取的地址小于最小开始地址，说明地址数据已损坏，重置activeparams
    if(addr_active_begin < ADDR_ACTIVE_BEGIN)
    {
        uint8_t zeros[sizeof(ActiveParams)];
        addr_active_begin = ADDR_ACTIVE_BEGIN;
        EEPROM_WriteRange(&eeprom, addr_active_begin, sizeof(ActiveParams), zeros);
    }
    // 读取active params
    EEPROM_ReadRange(&eeprom, addr_active_begin, sizeof(ActiveParams), p_actprm->bytes);
}


void save_config(union ConfigWriteBlock* p_cfg)
{
    EEPROM_release_WP(&eeprom);
    // ACK_polling_AT24C(pEEPROM);
    // write_array_AT24C(pEEPROM, 1, p_cfg->bytes, sizeof(ConfigWriteBlock));
    EEPROM_WriteRange(&eeprom, ADDR_CONFIG_BEGIN, sizeof(Config), p_cfg->bytes);
    EEPROM_WP(&eeprom);
}

void save_active_params(union ActiveWriteBlock* p_actprm)
{
    EEPROM_release_WP(&eeprom);
    uint16_t addr_active_begin;
#ifdef ADDR_ACTIVE_DYNAMIC
    p_actprm->body.n_cycles++;
    if(p_actprm->body.n_cycles >= 100)
    {
        p_actprm->body.n_cycles = 0;
        if((++page_active) >= n_pages[pEEPROM->capacity]) page_active = PAGE_ACTIVE_BEGIN; // the active params stroe after PAGE 16
        ACK_polling_AT24C(pEEPROM);
        write_AT24C(pEEPROM, 0, page_active);
    }
    ACK_polling_AT24C(pEEPROM);
    write_array_AT24C(pEEPROM, page_active, p_actprm->bytes, sizeof(ActiveWriteBlock));
#else // the address of active params is 
    addr_active_begin = ADDR_ACTIVE_BEGIN;
    EEPROM_WriteRange(&eeprom, addr_active_begin, sizeof(ActiveParams), p_actprm->bytes);
#endif
    EEPROM_WP(&eeprom);
}


bool config_check_valid(struct Config* p)
{
    uint8_t check = strncmp(p->sValid, VORTEX_VALID_STRING, sizeof(VORTEX_VALID_STRING)-1);
    return (check == 0);

}


bool set_config_variable_by_name(const char* name, const void* pV)
{
    const int N = (sizeof(configNameMapper) / sizeof(config_var_map_t));
    int i;
    bool found = false;
    // match the config entry by comparing names
    for(i = 0; i < N; ++i)
    {
        if(strcmp(configNameMapper[i].name, name) == 0)
        {
            found = true;
            break;
        }
    }
    if(found)
    {
        const config_var_map_t* pMap = &(configNameMapper[i]);
        switch (pMap->typ)
        {
        case CONFIG_VAR_U8:
            *(uint8_t*)(pMap->pV) = *(uint8_t*)pV;
            break;
        case CONFIG_VAR_I32:
            memcpy(pMap->pV, pV, sizeof(int));
            break;
        case CONFIG_VAR_BYTESTRING:
            strncpy((char*)(pMap->pV), (char*)pV, 31);
            break;
        }
    }
    return found;
}




config_var_map_t* get_config_variable_mapper_item_by_name(const char* name)
{
    const int N = (sizeof(configNameMapper) / sizeof(config_var_map_t));
    int i;
    bool found = false;
    config_var_map_t* pFound = NULL;
    // match the config entry by comparing names
    for(i = 0; i < N; ++i)
    {
        if(strcmp(configNameMapper[i].name, name) == 0)
        {
            found = true;
            pFound = &(configNameMapper[i]);
            break;
        }
    }
    return pFound;
}

bool get_config_variable_by_name(const char* name, void* buf)
{
    const int N = (sizeof(configNameMapper) / sizeof(config_var_map_t));
    int i;
    bool found = false;
    // match the config entry by comparing names
    for(i = 0; i < N; ++i)
    {
        if(strcmp(configNameMapper[i].name, name) == 0)
        {
            found = true;
            break;
        }
    }
    if(found)
    {
        const config_var_map_t* pMap = &(configNameMapper[i]);
        switch (pMap->typ)
        {
        case CONFIG_VAR_U8:
            *(uint8_t*)buf = *(uint8_t*)(pMap);
            break;
        case CONFIG_VAR_I32:
            memcpy(buf, pMap->pV, sizeof(int));
            break;
        case CONFIG_VAR_BYTESTRING:
            strncpy((char*)buf, (char*)(pMap->pV), 31);
            break;
        }
    }
    return found;
}

int get_config_string(char* buf, int lenbuf)
{
    const int N = (sizeof(configNameMapper) / sizeof(config_var_map_t));
    int i;
    char* p = buf;
    int nWritten = 0;
    int nW = 0;
    if (lenbuf <= 1)
        return 0;
    for(i = 0; i < N; ++i)
    {
        config_var_map_t *pMap = &(configNameMapper[i]);
        if(nWritten >= lenbuf - 1)
            break;
        switch(pMap->typ)
        {
            case CONFIG_VAR_U8:
            nW = snprintf(p, lenbuf - nWritten - 1, 
                "%s=%d", pMap->name, *((uint8_t*)(pMap->pV)));
            break;
            case CONFIG_VAR_I32:
            nW = snprintf(p, lenbuf - nWritten - 1, 
                "%s=%d", pMap->name, *((int*)(pMap->pV)));
            break;
            case CONFIG_VAR_BYTESTRING:
            nW = snprintf(p, lenbuf - nWritten - 1, 
                "%s=%s", pMap->name, ((char*)(pMap->pV)));
            break;
        }
        nWritten += nW;
        p += nW;
        if(nWritten < lenbuf - 1)
        {
            if(i == N-1)
            {
                *p = '\n';
            }
            else
            {
                *p = '&';
            }
            nWritten++;
            p++;
        }
    }
    buf[nWritten] = 0; // terminate the string
    return nWritten;
}
