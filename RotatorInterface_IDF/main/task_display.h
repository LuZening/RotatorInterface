#pragma once

#ifdef __cplusplus
extern "C" {
#endif
// #define LVGL_SIMULATOR

#include "MultiLang.h"

#include "lvgl.h"

// #define LVGL_SIMULATOR

#ifndef LVGL_SIMULATOR
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
    extern SemaphoreHandle_t mtxDisplay;
#endif
    extern lv_indev_t* lvIndev_RotEnc;
    extern uint16_t nRotEncCounter;
    extern bool lv_read_cb_rot_enc(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

    typedef enum {
        MENU_MAIN = 0,
        MENU_SETTINGS,
        MENU_NETWORK
    } menu_id_t;
    extern menu_id_t iCurrentMenu;
    extern int iCurrentSelectedMotor;
    extern void task_display(void* args);
#ifdef LVGL_SIMULATOR
    extern void test_lvgl_rotator_interface();
#endif
    typedef void(*MenuCb_t)(void);
    void init_lvgl_styles();
    void display_main_screen();
    extern uint32_t counterDisplaySleep;
    extern bool isDisplayOn;
    void display_turn_off();
    void display_turn_on();
/* Menu items BEGIN */
    void display_menu();
    void display_menu_item_network_settings();
    void display_menu_item_WiFi_status();
    void display_menu_item_ethernet_status();
/* Menu items END */
    void display_update_azu_degree(float degree);
    void display_update_elv_degree(float degree);
    void display_update_azu_mot_speed(int16_t speed100);
    void display_update_elv_mot_speed(int16_t speed100);
    

/* Menu items END */
#ifdef __cplusplus
}
#endif
