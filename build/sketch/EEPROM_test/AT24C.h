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

int page_sizes[] = {8, 8, 16, 16, 16, 32, 32};
int n_pages[] = {16, 32, 32, 64, 128, 128, 256};

struct EEPROM_AT24C
{
    enum AT24C_cap capacity;
    int pin_SCL;
    int pin_SDA;
    int pin_WP;
    unsigned char addr_chip;
} _EEPROM;

struct EEPROM_AT24C *pEEPROM = &_EEPROM;
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

void begin_AT24C(struct EEPROM_AT24C *pEEPROM, enum AT24C_cap capacity, int pin_SCL, int pin_SDA, int pin_WP, byte addr_chip)
{
    pEEPROM->capacity = capacity;
    pEEPROM->pin_SCL = pin_SCL;
    pEEPROM->pin_SDA = pin_SDA;
    pEEPROM->pin_WP = pin_WP;
    pEEPROM->addr_chip = addr_chip;
    pinMode(pin_SCL, OUTPUT);
    pinMode(pin_SDA, OUTPUT_OPEN_DRAIN);
    SET_PIN(pin_SCL, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pin_SDA, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
}

void WP_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    SET_PIN(pEEPROM->pin_WP, HIGH);
}
void release_WP_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    SET_PIN(pEEPROM->pin_WP, LOW);
}

void start_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    int SCL = pEEPROM->pin_SCL;
    int SDA = pEEPROM->pin_SDA;
    // start
    SET_PIN(SDA, HIGH);
    SET_PIN(SCL, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(SDA, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
}

void stop_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    int SCL = pEEPROM->pin_SCL;
    int SDA = pEEPROM->pin_SDA;
    SET_PIN(SDA, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(SCL, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(SDA, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
}

void send_ACK_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    SET_PIN(pEEPROM->pin_SDA, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pEEPROM->pin_SCL, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pEEPROM->pin_SCL, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pEEPROM->pin_SDA, HIGH);

}

bool ACK_polling_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    // send a dummy operation code
    byte op = get_op_AT24C(pEEPROM, 0);
    bool is_ACK = false;
    int i = 0;
    do
    {
        start_AT24C(pEEPROM);
        is_ACK = send_byte_AT24C(pEEPROM, op);
        i++;
    }while((!is_ACK) && (i <= 100));
    stop_AT24C(pEEPROM);
    return is_ACK;
}

bool send_byte_AT24C(struct EEPROM_AT24C *pEEPROM, byte data)
{
    int SCL = pEEPROM->pin_SCL;
    int SDA = pEEPROM->pin_SDA;
    bool is_ACK = false;
    int i = 0;
    // send the byte from MSB
    for (i = 7; i >= 0; --i)
    {
        SET_PIN(SCL, LOW);
        delayMicroseconds(AT24C_HOLDTIME);
        SET_PIN(SDA, (data >> i) & 0x01);
        delayMicroseconds(AT24C_HOLDTIME);
        SET_PIN(SCL, HIGH);
        delayMicroseconds(AT24C_HOLDTIME);
    }
    // ACK
    SET_PIN(SCL, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(SDA, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    while ((GET_PIN(SDA) == HIGH) && (i++) < 1000)
    {
        delayMicroseconds(10);
    }
    if(i < 1000) is_ACK = true;
    SET_PIN(SCL, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(SCL, LOW);
    //Serial.printf("content: %d, %d\r\n", data, is_ACK);
    return is_ACK;
}

byte recv_byte_AT24C(struct EEPROM_AT24C *pEEPROM)
{
    int i = 8;
    byte data = 0;
    SET_PIN(pEEPROM->pin_SCL, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    while (i--)
    {
        SET_PIN(pEEPROM->pin_SCL, HIGH);
        delayMicroseconds(AT24C_HOLDTIME);
        data <<= 1;
        data |= GET_PIN(pEEPROM->pin_SDA);
        SET_PIN(pEEPROM->pin_SCL, LOW);
        delayMicroseconds(AT24C_HOLDTIME);
    }
    return data;
}

byte get_op_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr)
{
    byte op;
    if (pEEPROM->capacity <= AT24C02)
    {
        op = 0b10100000 | ((pEEPROM->addr_chip << 1) & 0b1110);
    }
    else if (pEEPROM->capacity == AT24C04)
    {
        op = 0b10100000 | ((pEEPROM->addr_chip << 2) & 0b1100) | ((addr >> 7) & 0b10);
    }
    else if (pEEPROM->capacity == AT24C08)
    {
        op = 0b10100000 | ((pEEPROM->addr_chip << 3) & 0b1000) | ((addr >> 7) & 0b110);
    }
    else if (pEEPROM->capacity == AT24C16)
    {
        op = 0b10100000 | ((addr >> 7) & 0b1110);
    }
    else
    {
        op = 0b10100000 | ((pEEPROM->addr_chip << 1) & 0b1110);
    }
    return op;
}

void write_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr, byte data)
{
    start_AT24C(pEEPROM);
    send_byte_AT24C(pEEPROM, get_op_AT24C(pEEPROM, addr)); // write signal
    if (pEEPROM->capacity >= AT24C32)
    {
        // addr H
        send_byte_AT24C(pEEPROM, (addr >> 8) & 0xff);
    }
    // addr L
    send_byte_AT24C(pEEPROM, (addr & 0xff));
    // data
    send_byte_AT24C(pEEPROM, data);
    // stop
    stop_AT24C(pEEPROM);
}

byte read_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr)
{
    byte data;
    byte op = get_op_AT24C(pEEPROM, addr);
    start_AT24C(pEEPROM);
    send_byte_AT24C(pEEPROM, op); // write signal for address setting
    if(pEEPROM->capacity >= AT24C32)
    {
        // addr H
        send_byte_AT24C(pEEPROM, (addr >> 8) & 0xff);
    }
    // addr L
    send_byte_AT24C(pEEPROM, (addr & 0xff));
    start_AT24C(pEEPROM);
    send_byte_AT24C(pEEPROM, op | 0x01); // read signal
    data = recv_byte_AT24C(pEEPROM);
    stop_AT24C(pEEPROM);
    return data;
}


// only compatible with 24C64 and above
void read_array_AT24C(struct EEPROM_AT24C *pEEPROM, uint16_t addr, unsigned char* data, int n)
{
    int i;
    byte op = get_op_AT24C(pEEPROM, addr);
    start_AT24C(pEEPROM);
    send_byte_AT24C(pEEPROM, op); // write signal for address setting
    if(pEEPROM->capacity >= AT24C32)
    {
        // addr H
        send_byte_AT24C(pEEPROM, (addr >> 8) & 0xff);
    }
    // addr L
    send_byte_AT24C(pEEPROM, (addr & 0xff));
    start_AT24C(pEEPROM);
    send_byte_AT24C(pEEPROM, op | 0x01); // read signal
    data[0] = recv_byte_AT24C(pEEPROM);
    for(i=1; i<n; ++i)
    {
        // SEND ACK
        send_ACK_AT24C(pEEPROM);
        data[i] = recv_byte_AT24C(pEEPROM);
    }
    // NO ACK
    /*
    SET_PIN(pEEPROM->pin_SDA, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pEEPROM->pin_SCL, HIGH);
    delayMicroseconds(AT24C_HOLDTIME);
    SET_PIN(pEEPROM->pin_SCL, LOW);
    delayMicroseconds(AT24C_HOLDTIME);
    */
    wdt_reset(); // feed the watch dog
    stop_AT24C(pEEPROM);
}

void write_array_AT24C(struct EEPROM_AT24C *pEEPROM, int page, unsigned char* data, int n)
{
    int i = 0;
    int page_size = page_sizes[(int)(pEEPROM->capacity)];
    // total number of pages the chip has
    int max_page = n_pages[(int)(pEEPROM->capacity)];
    // number of pages we will need to write data
    int n_page = n / page_size + ((n % page_size)?(1):(0));
    int addr = page * page_size;
    if (page + n_page > max_page) return;

    while(i < n)
    {
        // change page
        if(i % page_size == 0)
        {
            if(i > 0) 
            {
                stop_AT24C(pEEPROM);
                ACK_polling_AT24C(pEEPROM);
                //Serial.println(addr);
                addr += page_size;
                wdt_reset();// feed the watchdog
            }
            start_AT24C(pEEPROM);
            send_byte_AT24C(pEEPROM, get_op_AT24C(pEEPROM, addr)); // write
            if(pEEPROM->capacity >= AT24C32)
            {
                // addr H
                send_byte_AT24C(pEEPROM, (addr >> 8) & 0xff);
            }
            // addr L
            send_byte_AT24C(pEEPROM, (addr & 0xff));
        }
        send_byte_AT24C(pEEPROM, data[i]);
        ++i;
    }
    stop_AT24C(pEEPROM);

}
#endif