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
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "AT24C.h"
#include "config.h"
#include "RotSensor.h"
#include "RotTask.h"

// USE ESP32 internal EMAC
#define USE_ETHERNET
#include "my_network.h"
#include "webserver.h"

/* task headers */
#include "task_display.h"

#define tag "user_main"

#define PIN_595_SER 5
#define PIN_595_SRCLK 2
#define PIN_595_RCLK 4
#define PIN_595_OE 16

#include "SPWM.h"

/* OS objects BEGIN */
// main event group to respond to Reset/EEPROM writing
EventGroupHandle_t evtgrpMain = NULL;
QueueHandle_t qRotTasks;
/* OS objects END */


void reset_async()
{
    xEventGroupSetBits(evtgrpMain, SIGNAL_RESET);
}

void write_to_EEPROM_async()
{
    xEventGroupSetBits(evtgrpMain, SIGNAL_SAVE_CONFIG);
}
/* Initializers BEGIN    */




/* Initializers BEGIN    */

void setup(void)
{
    evtgrpMain = xEventGroupCreate();
    init_network(); // init Event Loop
    /* init 74595 */
    begin_595(p595, PIN_595_SRCLK, PIN_595_RCLK, PIN_595_OE, PIN_595_SER);
    for(uint8_t i = 0; i < 8; ++i)
    {
        write_595(p595, i, 0);
    }
    /* Init NVS BEGIN */
    ESP_ERROR_CHECK(nvs_flash_init());
    /* Init webserver BEGIN */
    ESP_LOGD(tag, "Initializing webserver");
    init_webserver();
    ESP_LOGD(tag, "Webserver initialized");
    /* Init webserver END */
    /* Init EEPROM BEGIN */
    // note: I2C driver has been installed  in display driver, so do not do that twice (initI2C = false)
    // EEPROM_init(true, &eeprom, AT24C64_SIZE, 
    //             EEPROM_I2C_PORT, 
    //             EEPROM_ADDR, 
    //             EEPROM_GPIO_SDA, EEPROM_GPIO_SCL, EEPROM_GPIO_WP);
    // read config
    load_config(&cfg);
    // config invalid, start as AP
    bool initConfig = false;
    if(!config_check_valid(&cfg))
    {
        ESP_LOGD(tag, "Config not found");
        init_config(&cfg);
        initConfig = true;
        ESP_LOGD(tag, "Config initialized");
    }

    /* init display */
    ESP_LOGD(tag, "starting task_display");
    mtxDisplay = xSemaphoreCreateMutex();
    xTaskCreate(task_display, "task_display", 3072, NULL, 2, NULL);
    if(initConfig)
    {
        // config valid, connect to wifi station
        // when config is not correct, start wifi AP to assist user
        (init_wifi(true, WIFI_SSID, NULL));
        ESP_LOGD(tag, "WiFi AP enabled");
    }
    else
    {
        if(cfg.use_WiFi || (!cfg.use_Ethernet && !cfg.use_RS485)) 
        {
            // prepare the SSID and password of the AP to connect 
            char ssid[LEN_SSID_MAX + 1], password[LEN_WIFI_PASSWORD_MAX + 1];
            strncpy(ssid, cfg.WiFi_SSID, sizeof(ssid)-1);
            ssid[sizeof(ssid) - 1] = 0;
            strncpy(password, cfg.WiFi_password, sizeof(password) - 1);
            password[sizeof(password) - 1] = 0;
            bool r = init_wifi(false, ssid, password);
            // connect to target AP succeeded
            if(r)
            {
                ESP_LOGD(tag, "WiFi STA connection established");
            }
            // if failed, start AP
            else
            {
                ESP_LOGD(tag, "WiFi STA connection to %s failed, starting AP instead", ssid);
                init_wifi(true, WIFI_SSID, NULL);
            }
        }

        if(cfg.use_Ethernet)
        {
            ESP_LOGD(tag, "Use Ethernet instead of WiFi...");
            init_ethernet();
        }
        // TODO: use RS485
    }
    /* Init EEPROM END */
    /* Init Ethernet */
    vTaskDelay(pdMS_TO_TICKS(500)); // wait for PHY to reset

}

void start_tasks(void)
{
    /* OS signaling objects BEGIN */
	qRotTasks = xQueueCreate(16, sizeof(RotTask_t));
    /* OS signaling objects END */
    init_rot_sensor_task();
    ESP_LOGD(tag, "starting task_rot_senor");
    xTaskCreate(task_rot_sensor, "task_rot_sen", 3072, NULL, 4, NULL);
    ESP_LOGD(tag, "starting task_rot_tasks");
    xTaskCreate(task_rot_tasks, "task_rot_tasks", 2048 , NULL, 4, NULL);
    init_human_input_task();
    ESP_LOGD(tag, "starting task_human_input");
    xTaskCreate(task_human_input, "task_human_in", 2048 + 256, NULL, 3, NULL);

}

void app_main(void)
{
    portMUX_TYPE muxCritical;
    setup();
    start_tasks();
    ESP_LOGD(tag, "All tasks started");
    int sigReset;
    // handle EEPROM saving request
    while(1)
    {
        EventBits_t bits = xEventGroupWaitBits(evtgrpMain, 0b11, pdTRUE, pdFALSE, portMAX_DELAY);
        if((bits & SIGNAL_SAVE_CONFIG) > 0)  // save config on EEPROM
        {
            ESP_LOGD(tag, "Writing EEPROM...");
            // taskENTER_CRITICAL(&muxCritical);
            save_config(&cfg);
            // taskEXIT_CRITICAL(&muxCritical);
            ESP_LOGD(tag, "Writing EEPROM DONE!");
        }
        if((bits & SIGNAL_RESET) > 0) // reset
        {
            ESP_LOGD(tag, "received request for RESET...");
            abort();
        }
        if(bits & SIGNAL_SPWM_START_UNIT_TEST > 0)
        {
            ESP_LOGD(tag, "starting SPWM unit test...");
            SPWM_unit_test_start();
        }
        if(bits & SIGNAL_SPWM_END_UNIT_TEST > 0)
        {
            ESP_LOGD(tag, "ending SPWM unit test...");
            SPWM_stop(&spwm);
        }
    }
    
        // taskEXIT_CRITICAL(&muxCritical);
}


// char bufTaskList[1280];
// void print_task_status()
// {
//     vTaskList(bufTaskList);
//     ESP_LOGD("", "%s", bufTaskList);