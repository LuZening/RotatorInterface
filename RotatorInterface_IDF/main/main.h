#pragma once

#include "sdkconfig.h"
#include "AT24C.h"
#include "RotSensor.h"
#include "freertos/FreeRTOS.h"
#include "motor.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "lvgl.h"
#include "MultiLang.h"
// EEPROM
#define EEPROM_ADDR 0x00
#define EEPROM_I2C_PORT I2C_NUM_0
#define EEPROM_GPIO_SDA 33
#define EEPROM_GPIO_SCL 32
#define EEPROM_GPIO_WP 0
extern EEPROM_t eeprom;
// WiFi
#define WIFI_SSID "VORTEX3"
#define LEN_WIFI_PASSWORD_MAX 32
#define LEN_SSID_MAX 32


// RS485
#define RS485_GPIO_RW 0

// output board type
#define OUTPUT_BOARD_TRIAC_RELAY
// main event group
#define SIGNAL_SAVE_CONFIG 0b10U
#define SIGNAL_RESET 0b01U
extern EventGroupHandle_t evtgrpMain;

/* Language */
extern multi_lang_t lang;

extern QueueHandle_t qMailboxReset;

/* Public variables defined in task_RotSensors.c BEGIN */
// define RotSensors in task_RotSensor
extern SemaphoreHandle_t mutexRotSensor;
extern RotSensor arrRotSensors[];
extern Motor arrMotors[];
extern int arrDegreeValues[];
extern int arrRotADCRawValues[];
extern int arrObservedMotSpeeds[]; // deg / sec
extern int arrLimits[];
/* Public variables defined in task_RotSensors.c END */

/* tasks BEGIN */
extern void reset_async(); // defined in main task
void write_to_EEPROM_async();
extern void init_rot_sensor_task();
extern void task_rot_sensor(void* args);
extern QueueHandle_t qRotTasks;
extern void task_rot_tasks(void* args);
typedef enum{
    BUTTON_SET = 0,
    BUTTON_CW,
    BUTTON_CCW,
} HumanInputType_t;
extern QueueHandle_t qHumanInputOperations;
extern void init_human_input_task();
extern void task_human_input(void* args);
extern char bufTaskList[];
void print_task_status();
/* tasks END */
int get_current_degree_value(int i);