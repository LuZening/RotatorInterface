#pragma once


#include "driver/i2c.h"

#define I2C_FREQUENCY   100000
#define ACK_CHECK_EN    0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS   0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL         0x0     /*!< I2C ack value */
#define NACK_VAL        0x1     /*!< I2C nack value */

#define AT24C02_SIZE 2
#define AT24C16_SIZE 16
#define AT24C32_SIZE 32
#define AT24C64_SIZE 64

typedef struct {
	uint16_t _i2c_port;
	uint16_t _chip_addr;
	uint16_t _size;
	uint16_t _bytes;
	uint16_t pinWP;
} EEPROM_t;

extern EEPROM_t eeprom;

esp_err_t EEPROM_init(EEPROM_t * dev, int16_t size, i2c_port_t i2c_port, int chip_addr, int i2c_gpio_sda, int i2c_gpio_scl, int pin_WP);
// get maximum  bytes
uint16_t EEPROM_MaxAddress(EEPROM_t * dev);
//static esp_err_t ReadReg8(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint8_t data_addr, uint8_t * data);
//static esp_err_t WriteReg8(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint8_t data_addr, uint8_t data);
//static esp_err_t ReadReg16(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint16_t data_addr, uint8_t * data);
//static esp_err_t WriteReg16(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint16_t data_addr, uint8_t data);
esp_err_t EEPROM_ReadByte(EEPROM_t * dev, uint16_t data_addr, uint8_t * data);
esp_err_t EEPROM_ReadRange(EEPROM_t * dev, uint16_t data_addr,  uint8_t * data,  uint16_t len);
void EEPROM_release_WP(EEPROM_t *dev);
esp_err_t EEPROM_WriteByte(EEPROM_t * dev, uint16_t data_addr, uint8_t data);
esp_err_t EEPROM_WriteRange(EEPROM_t * dev, uint16_t data_addr, uint16_t len, const uint8_t* data);
void EEPROM_WP(EEPROM_t *dev);

