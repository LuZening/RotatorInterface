#ifndef __AT24C_H_
#define __AT24C_H_

#include <Arduino.h>
#include "Lib595.h"
#define GET_PIN(p) digitalRead(p)
#define AT24C_HOLDTIME 6
#define PAGE2ADDR(pEEPROM, p) ( (p) * page_sizes[(pEEPROM)->capacity] )


enum AT24C_cap
{
    AT24C01=0,
    AT24C02,
    AT24C04,
    AT24C08,
    AT24C16,
    AT24C32,
    AT24C64
};

extern const int page_sizes[];
extern const int n_pages[];

struct EEPROM_AT24C
{
    enum AT24C_cap capacity;
    int pin_SCL;
    int pin_SDA;
    int pin_WP;
    unsigned char addr_chip;
};

extern struct EEPROM_AT24C *pEEPROM;

void begin_AT24C(struct EEPROM_AT24C *pEEPROM, enum AT24C_cap capacity, int pin_SCL, int pin_SDA, int pin_WP, byte addr_chip);
void WP_AT24C(struct EEPROM_AT24C *pEEPROM);
void release_WP_AT24C(struct EEPROM_AT24C *pEEPROM);
void start_AT24C(struct EEPROM_AT24C *pEEPROM);
void stop_AT24C(struct EEPROM_AT24C *pEEPROM);
void send_ACK_AT24C(struct EEPROM_AT24C *pEEPROM);
bool ACK_polling_AT24C(struct EEPROM_AT24C *pEEPROM);
bool send_byte_AT24C(struct EEPROM_AT24C *pEEPROM, byte data);
byte recv_byte_AT24C(struct EEPROM_AT24C *pEEPROM);
byte get_op_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr);
void write_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr, byte data);
byte read_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr);
void read_array_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr, unsigned char* data, int n);
void write_array_AT24C(struct EEPROM_AT24C *pEEPROM, int page, unsigned char* data, int n);

#endif