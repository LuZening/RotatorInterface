#line 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\Config.cpp"
#include "Config.h"


union ConfigWriteBlock _cfg;
union ConfigWriteBlock* p_cfg = &_cfg;
union ActiveWriteBlock _actprm;
union ActiveWriteBlock* p_actprm = &_actprm;
byte page_active;

void load_config(union ConfigWriteBlock* p_cfg)
{
    ACK_polling_AT24C(pEEPROM);
    read_array_AT24C(pEEPROM, PAGE2ADDR(pEEPROM, PAGE_CONFIG), p_cfg->bytes, sizeof(ConfigWriteBlock));
    Serial.printf("ssid=%s, passwd=%s\r\n", p_cfg->body.s_ssid,p_cfg->body.s_password);
}

void load_active_params(union ActiveWriteBlock* p_actprm)
{
    // find the page number
    ACK_polling_AT24C(pEEPROM);
    page_active = read_AT24C(pEEPROM, 0);
    // Serial.print("EEPROM PAGE ACTIVE:");
    // Serial.println(page_active);
    if(page_active < PAGE_ACTIVE_BEGIN)
    {
        page_active = PAGE_ACTIVE_BEGIN;
        write_AT24C(pEEPROM, 0, page_active);
        ACK_polling_AT24C(pEEPROM);
    }
    read_array_AT24C(pEEPROM, PAGE2ADDR(pEEPROM, page_active), p_actprm->bytes, sizeof(ActiveWriteBlock));
}

/* EEPROM arrangement AT24C64 32Bytes * 256Pages */
/* PAGE 0: PAGE TO FIND ACTIVE PARAMS - 1Byte   */
/* PAGE 1-15: CONFIG
/* PAGE 16- : ACTIVE PARAMS
*/  

void save_config(union ConfigWriteBlock* p_cfg)
{
    release_WP_AT24C(pEEPROM);
    ACK_polling_AT24C(pEEPROM);
    write_array_AT24C(pEEPROM, 1, p_cfg->bytes, sizeof(ConfigWriteBlock));
    WP_AT24C(pEEPROM);
}

void save_active_params(union ActiveWriteBlock* p_actprm)
{
    release_WP_AT24C(pEEPROM);
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
    WP_AT24C(pEEPROM);
}

