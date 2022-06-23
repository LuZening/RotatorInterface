/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "AT24C.h"
#include "config.h"
#include "RotSensor.h"
#include "RotTask.h"

// USE ESP32 internal EMAC
#define USE_ETHERNET
#include "my_network.h"
#include "webserver.h"

#define tag "user_main"



/* OS objects BEGIN */
QueueHandle_t qMailboxReset;
QueueHandle_t qRotTasks;
/* OS objects END */


/* Initializers BEGIN    */




/* Initializers BEGIN    */

void setup(void)
{
    /* Init Ethernet */
    ethernet_init();
    /* Init EEPROM BEGIN */
    EEPROM_init(&eeprom, AT24C02_SIZE, 
                EEPROM_I2C_PORT, 
                EEPROM_ADDR, 
                EEPROM_GPIO_SDA, EEPROM_GPIO_SCL, EEPROM_GPIO_WP);
    // read config
    load_config((union ConfigWriteBlock*)&cfg);
    if(!config_check_valid(&cfg))
    {
        init_config(&cfg);
		// when config is not correct, start wifi AP to assist user
		init_wifi(true);
    }
    /* Init EEPROM END */
    
    /* Init webserver BEGIN */
    init_webserver();
    /* Init webserver END */


}

void start_tasks(void)
{
    /* OS signaling objects BEGIN */
	qMailboxReset = xQueueCreate(1, sizeof(int));
	qRotTasks = xQueueCreate(16, sizeof(RotTask_t));
    mutexRotSensor = xSemaphoreCreateMutex();
    /* OS signaling objects END */
    xTaskCreate(task_rot_sensor, "task_rot_sensor", 1024, mutexRotSensor, 1, NULL);
}

void app_main(void)
{
    portMUX_TYPE muxCritical;
    setup();
    start_tasks();
    int sigReset;
    if(xQueuePeek(qMailboxReset, &sigReset, 0) != errQUEUE_EMPTY)
    {
        // taskENTER_CRITICAL(&muxCritical);
        abort();
        // taskEXIT_CRITICAL(&muxCritical);
    }

}
