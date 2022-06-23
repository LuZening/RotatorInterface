#pragma once

#include "sdkconfig.h"
#include "AT24C.h"
#include "RotSensor.h"
#include "freertos/FreeRTOS.h"
#include "motor.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// EEPROM
#define EEPROM_ADDR 0x00
#define EEPROM_I2C_PORT I2C_NUM_0
#define EEPROM_GPIO_SDA 33
#define EEPROM_GPIO_SCL 32
#define EEPROM_GPIO_WP 0
extern EEPROM_t eeprom;

// RS485
#define RS485_GPIO_RW 0

// output board type
#define OUTPUT_BOARD_TRIAC_RELAY

extern QueueHandle_t qMailboxReset;
extern QueueHandle_t qRotTasks;

// define RotSensors in task_RotSensor
extern SemaphoreHandle_t mutexRotSensor;
extern RotSensor arrRotSensors[];
extern Motor arrMotors[];

/* tasks BEGIN */
void task_rot_sensor(void* args);
void task_(void* args);

/* tasks END */
int get_current_degree_values(int i);