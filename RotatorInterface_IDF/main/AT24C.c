/*
Driver for reading and writing data to 24Cxx external I2C EEPROMs.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "at24c.h"

#define tag "at24c"

EEPROM_t eeprom;


esp_err_t EEPROM_init(bool initI2C, EEPROM_t * dev, int16_t size, i2c_port_t i2c_port, int chip_addr, int i2c_gpio_sda, int i2c_gpio_scl, int pin_WP)
{
	dev->pinWP = pin_WP;
	gpio_set_direction(pin_WP, GPIO_MODE_OUTPUT_OD);
	gpio_set_pull_mode(pin_WP, GPIO_PULLUP_ONLY);
	EEPROM_WP(dev);
	dev->_i2c_port = i2c_port;
	dev->_chip_addr = chip_addr;
	dev->_size = size;
	dev->_bytes = 128 * size;
	esp_err_t ret = ESP_OK;
	if(initI2C)
	{
		i2c_config_t conf = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = i2c_gpio_sda,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = i2c_gpio_scl,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = I2C_FREQUENCY
		};
		ret = i2c_param_config(i2c_port, &conf);
		ESP_LOGD(tag, "i2c_param_config=%d", ret);
		if (ret != ESP_OK) return ret;
		ret = i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
		ESP_LOGD(tag, "i2c_driver_install=%d", ret);
	}
	return ret;
}


uint16_t EEPROM_MaxAddress(EEPROM_t * dev) {
	return dev->_bytes;
}


static esp_err_t ReadReg8(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint8_t data_addr, uint8_t * data)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, data, NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return ret;
}


static esp_err_t WriteReg8(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint8_t data_addr, uint8_t data)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	usleep(100);
	return ret;
}


static esp_err_t ReadReg16(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint16_t data_addr, uint8_t * data)
{
	uint8_t high_addr = (data_addr >> 8) & 0xff;
	uint8_t low_addr = data_addr & 0xff;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, high_addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, low_addr, ACK_CHECK_EN);
	// i2c_master_start(cmd);
	// i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
	// i2c_master_read_byte(cmd, data, NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(dev->_i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return ret;
}


static esp_err_t WriteReg16(EEPROM_t * dev, i2c_port_t i2c_port, int chip_addr, uint16_t data_addr, uint8_t data)
{
	uint8_t high_addr = (data_addr >> 8) & 0xff;
	uint8_t low_addr = data_addr & 0xff;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, high_addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, low_addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	usleep(100);
	return ret;
}


esp_err_t EEPROM_ReadByte(EEPROM_t * dev, uint16_t data_addr, uint8_t * data)
{
	if (data_addr > dev->_bytes) return 0;
	esp_err_t err;
	if (dev->_size < 32) {
		int blockNumber = data_addr / 256;
		uint16_t _data_addr = data_addr - (blockNumber * 256);
		int _chip_addr = dev->_chip_addr + blockNumber;
		err = ReadReg8(dev, dev->_i2c_port, _chip_addr, _data_addr, data);
		ESP_LOGD(tag, "ReadRom8 _chip_addr=%x _data_addr=%x _data=%x _err=%d", _chip_addr, _data_addr, *data, err);
	} else {
		int _chip_addr = dev->_chip_addr;
		err = ReadReg16(dev, dev->_i2c_port, _chip_addr, data_addr, data);
		ESP_LOGD(tag, "ReadRom16 _chip_addr=%x _data_addr=%x _data=%x _err=%d", _chip_addr, data_addr, *data, err);
		
	}
	return err;
}


esp_err_t EEPROM_ReadRange(EEPROM_t * dev, uint16_t data_addr,  uint8_t * data,  uint16_t len)
{
	esp_err_t r = ESP_OK;
	while(len--)
	{
		EEPROM_ReadByte(dev, data_addr++, data++);
	}
	return r;
}

void EEPROM_release_WP(EEPROM_t *dev)
{
	gpio_set_level(dev->pinWP, 0);
}

esp_err_t EEPROM_WriteByte(EEPROM_t * dev, uint16_t data_addr, uint8_t data)
{
	if (data_addr > dev->_bytes) return 0;
	esp_err_t err;
	if (dev->_size < 32) {
		int blockNumber = data_addr / 256;
		uint16_t _data_addr = data_addr - (blockNumber * 256);
		int _chip_addr = dev->_chip_addr + blockNumber;
		err = WriteReg8(dev, dev->_i2c_port, _chip_addr, _data_addr, data);
		ESP_LOGD(tag, "WriteRom8 _chip_addr=%x _data_addr=%x _data=%x _err=%d", _chip_addr, _data_addr, data, err);
		return err;
	} else {
		int _chip_addr = dev->_chip_addr;
		err = WriteReg16(dev, dev->_i2c_port, _chip_addr, data_addr, data); 
		ESP_LOGD(tag, "WriteRom16 _chip_addr=%x _data_addr=%x _data=%x _err=%d", _chip_addr, data_addr, data, err);
	}
	return err;
}


esp_err_t EEPROM_WriteRange(EEPROM_t * dev, uint16_t data_addr, uint16_t len, const uint8_t* data)
{
	esp_err_t r = ESP_OK;
	while(len--)
	{
		EEPROM_WriteByte(dev, data_addr++, *(data++));
	}
	return r;
}


void EEPROM_WP(EEPROM_t *dev)
{
	gpio_set_level(dev->pinWP, 1);
}