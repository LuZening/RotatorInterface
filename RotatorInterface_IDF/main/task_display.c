#include "task_display.h"
#include "lvgl.h"
#ifndef LVGL_SIMULATOR
#define USE_MUTEX_IN_DISPLAY
#include "lvgl_helpers.h"
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "my_network.h"
   
#else
#include <stdio.h>
#include "esp_netif_ip_addr.h" // you need to repliacate a stand-alone version of the header file to avoid dependency to ESP-IDF
#endif
#define tag "display"

/* Input Group management */
#ifndef LVGL_SIMULATOR
lv_indev_t* lvIndev_RotEnc = NULL;
#endif
// Current objects to control by external inputs
// when modify, must be protected by Mutex
#define N_GROUP_STACK_SIZE 16 // 最多可以容纳16层嵌套菜单
static lv_group_t* lvGroups[N_GROUP_STACK_SIZE] = { NULL };
static uint8_t idxGroups = 0; // 指向当前正在操作的group的后一个空位

static lv_group_t* get_current_group()
{
    if (idxGroups <= N_GROUP_STACK_SIZE && idxGroups > 0)
        return lvGroups[-1];
    else
        return NULL;
}

static bool register_group(lv_group_t* p)
{
    if (idxGroups < N_GROUP_STACK_SIZE)
    {
        lvGroups[idxGroups++] = p;
        lv_indev_set_group(lvIndev_RotEnc, p);
        return true;
    }
    else
        return false;
}

static bool deregister_group(lv_group_t* p)
{
    bool r = false;
    uint8_t i, j;
    if (p == NULL)
        return false;
    for (i = 0; i < idxGroups; ++i)
    {
        if (lvGroups[i] == p)
        {
            lv_group_del(lvGroups[i]);
            lvGroups[i] = NULL;
            if (idxGroups - i == 1) // the group to be deregistered is the focused group now, then use the previous group
            {
                // look backward to find the first non-NULL group pointer, and promote it as the focused group
                for (j = i; j > 0; --j)
                {
                    if (lvGroups[j - 1] != NULL)
                    {
                        idxGroups = j;
                        lv_indev_set_group(lvIndev_RotEnc, lvGroups[j - 1]);
                        break;
                    }
                }
                // if not found, set idx to 0, set current focused group to NULL
                if (j == 0)
                {
                    idxGroups = 0;
                    lv_indev_set_group(lvIndev_RotEnc, NULL);
                }
            }
            r = true;
            ////break;
        }
    }
    return r;
}



/* language */
multi_lang_t lang = LANG_EN_US;

#ifndef LVGL_SIMULATOR
SemaphoreHandle_t mtxDisplay;
lv_indev_t* lvIndev_Keys;
#endif
lv_color_t frameBufferGUI[LV_HOR_RES_MAX * LV_VER_RES_MAX];
menu_id_t iCurrentMenu = MENU_MAIN; // main screen
int iCurrentSelectedMotor = 0; // the id of the selected motor 0 ~ N_MOTs-

DEFINE_MULTI_LANG_STR_2(STR_BACK, "返回", "Back");

static void lv_tick_task(void *arg) {
    lv_tick_inc(1);
}


void display_turn_off()
{
    if(!isDisplayOn) return;
    isDisplayOn = false;
#ifndef LVGL_SIMULATOR
    ssd1306_sleep_in();
#endif
}

void display_turn_on()
{
#ifndef LVGL_SIMULATOR
    if(!isDisplayOn)
        ssd1306_sleep_out();
#endif
    isDisplayOn = true;
}

uint32_t counterDisplaySleep = 0;
bool isDisplayOn = true;
void task_display(void* args)
{
#ifndef LVGL_SIMULATOR
    static TickType_t tickLastWakeUp;
    tickLastWakeUp = xTaskGetTickCount();
    /* setup */
    lv_init();
    lvgl_driver_init();
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, frameBufferGUI, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);
    // register LVGL input drivers
    // static lv_indev_drv_t lvIndevKeys;
    // lv_indev_drv_init(&lvIndevKeys);
    // lvIndevKeys.read_cb = lv_read_cb_keys;
    // lvIndevKeys.type = LV_INDEV_TYPE_KEYPAD;
    // lvIndev_Keys = lv_indev_drv_register(&lvIndevKeys);
    static lv_indev_drv_t lvIndevRotEnc;
    lv_indev_drv_init(&lvIndevRotEnc);
    lvIndevRotEnc.read_cb = lv_read_cb_rot_enc;
    lvIndevRotEnc.type = LV_INDEV_TYPE_ENCODER;
    lvIndev_RotEnc= lv_indev_drv_register(&lvIndevRotEnc);
    // use ESP timer to tick LVGL heartbeat (each 1ms)
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "LVGL_tick"
    };
    esp_timer_handle_t periodic_timer;
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 1000U); // tick LVGL each 1ms
    // display inital screen
    init_lvgl_styles();
    display_main_screen();
    ESP_LOGD(tag, "task initialized");
    /* super loop */
    static int32_t arrDegreeValuesOld[N_POT_INPUTS] = {INT32_MIN};
    while(1)
    {
        /* update values */
        if(cfg.N_POTs > 0 && arrDegreeValues[0] != arrDegreeValuesOld[0])
        {
            display_update_azu_degree(arrDegreeValues[0]);
            arrDegreeValuesOld[0] = arrDegreeValues[0];
        }
        if(cfg.N_POTs > 1 && arrDegreeValues[1] != arrDegreeValuesOld[1])
        {
            display_update_elv_degree(arrDegreeValues[1]);
            arrDegreeValuesOld[1] = arrDegreeValues[1];
        }
        xSemaphoreTake(mtxDisplay, portMAX_DELAY);
        lv_task_handler();
        xSemaphoreGive(mtxDisplay);
        // display auto turnoff counter
        TickType_t ticksSinceLastWakeUp = xTaskGetTickCount() - tickLastWakeUp;
        tickLastWakeUp = xTaskGetTickCount();
        if(isDisplayOn)
        {
            counterDisplaySleep += ticksSinceLastWakeUp;
            if(counterDisplaySleep >= pdMS_TO_TICKS(60000)) // turn off the display after an idle of 60secs
            {
                display_turn_off();
                counterDisplaySleep = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
#endif

}

#ifdef LVGL_SIMULATOR
// for PC simulation only
void test_lvgl_rotator_interface()
{
    init_lvgl_styles();
    display_main_screen();
}
#endif

/* LVGL theme  */


lv_style_t lvStyleSolidCont;
lv_style_t lvStyleTransparentCont;
lv_style_t lvStyleTextSmall;
lv_style_t lvStyleTextLarge;
lv_style_t lvStyleBtnLarge;
lv_style_t lvStyleBtnSmall;
static void theme_apply_cb(lv_theme_t * th, lv_obj_t * obj, lv_theme_style_t name);
void init_lvgl_styles()
{
    /*Get the current theme (e.g. material). It will be the base of the custom theme.*/   
    lv_theme_t * base_theme = lv_theme_get_act();
   
    /*Initialize a custom theme*/
    static lv_theme_t custom_theme;                         /*Declare a theme*/
    lv_theme_copy(&custom_theme, base_theme);               /*Initialize the custom theme from the base theme*/                           
    lv_theme_set_apply_cb(&custom_theme, theme_apply_cb);  /*Set a custom theme apply callback*/
    lv_theme_set_base(&custom_theme, base_theme);            /*Set the base theme of the csutom theme*/

    lv_style_init(&lvStyleSolidCont);
    lv_style_set_border_width(&lvStyleSolidCont, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&lvStyleSolidCont, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&lvStyleSolidCont, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_pad_all(&lvStyleSolidCont, LV_STATE_DEFAULT, 0);

    lv_style_init(&lvStyleTransparentCont);
    lv_style_set_border_width(&lvStyleTransparentCont, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&lvStyleTransparentCont, 0xff, LV_OPA_TRANSP);
    lv_style_set_pad_all(&lvStyleTransparentCont, LV_STATE_DEFAULT, 0);

    lv_style_init(&lvStyleTextSmall);
    lv_style_copy(&lvStyleTextSmall, &lvStyleSolidCont);
    lv_style_set_border_width(&lvStyleTextSmall, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_all(&lvStyleTextSmall, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&lvStyleTextSmall, LV_STATE_DEFAULT, &lv_font_unscii_8);

    lv_style_init(&lvStyleTextLarge);
    lv_style_copy(&lvStyleTextLarge, &lvStyleTextSmall);
    lv_style_set_pad_all(&lvStyleTextLarge, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&lvStyleTextLarge, LV_STATE_DEFAULT, &lv_font_unscii_16);
    
    lv_style_init(&lvStyleBtnLarge);
    lv_style_copy(&lvStyleBtnLarge, &lvStyleTextLarge);
    lv_style_set_border_color(&lvStyleBtnLarge, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_style_set_border_opa(&lvStyleBtnLarge, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_opa(&lvStyleBtnLarge, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_border_width(&lvStyleBtnLarge, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&lvStyleBtnLarge, LV_STATE_FOCUSED, 2);
    lv_style_set_radius(&lvStyleBtnLarge, LV_STATE_DEFAULT, 0);
    
    lv_style_init(&lvStyleBtnSmall);
    lv_style_copy(&lvStyleBtnSmall, &lvStyleTextSmall);
    lv_style_set_border_color(&lvStyleBtnSmall, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_style_set_border_opa(&lvStyleBtnSmall, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_opa(&lvStyleBtnSmall, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_border_width(&lvStyleBtnSmall, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&lvStyleBtnSmall, LV_STATE_FOCUSED, 2);
    lv_style_set_radius(&lvStyleBtnSmall, LV_STATE_DEFAULT, 0);

    lv_theme_set_act(&custom_theme);
}

/*Add a custom apply callback*/
static void theme_apply_cb(lv_theme_t * th, lv_obj_t * obj, lv_theme_style_t name)
{
    lv_style_list_t * list;

    switch(name) {
        case LV_THEME_BTN:
            lv_obj_clean_style_list(obj, LV_BTN_PART_MAIN);
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &lvStyleBtnLarge);
            lv_obj_set_size(obj, 48, 28);
            break;
        case LV_THEME_LABEL:
            lv_obj_clean_style_list(obj, LV_LABEL_PART_MAIN);
            list = lv_obj_get_style_list(obj, LV_LABEL_PART_MAIN);
            _lv_style_list_add_style(list, &lvStyleTextSmall);
            break;
        case LV_THEME_PAGE:
            lv_obj_clean_style_list(obj, LV_PAGE_PART_BG);
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_BG);
            _lv_style_list_add_style(list, &lvStyleSolidCont);
            lv_page_set_scrlbar_mode(obj, LV_SCRLBAR_MODE_HIDE);
            lv_page_set_edge_flash(obj, false);
            lv_obj_set_size(obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
            break;
        case LV_THEME_MSGBOX:
            lv_obj_clean_style_list(obj, LV_MSGBOX_PART_BG);
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BG);
            _lv_style_list_add_style(list, &lvStyleSolidCont);
            lv_obj_clean_style_list(obj, LV_MSGBOX_PART_BTN);
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN);
            _lv_style_list_add_style(list, &lvStyleBtnSmall);
            break;
        case LV_THEME_MSGBOX_BTNS:
            lv_obj_clean_style_list(obj, LV_MSGBOX_PART_BTN);
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN);
            _lv_style_list_add_style(list, &lvStyleBtnSmall);
            break;
        case LV_THEME_BAR:
            lv_obj_add_style(obj, LV_BAR_PART_BG, &lvStyleSolidCont);
            lv_obj_set_style_local_border_width(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
            lv_obj_add_style(obj, LV_BAR_PART_INDIC, &lvStyleSolidCont);
            lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_INDIC,  LV_STATE_DEFAULT, LV_COLOR_WHITE);
            
            lv_bar_set_type(obj, LV_BAR_TYPE_SYMMETRICAL);
            lv_bar_set_anim_time(obj, LV_ANIM_OFF);
            lv_bar_set_value(obj, 0, LV_ANIM_OFF);
            break;
        default:
            break;
    }
}



/* Main screen widgets */
lv_group_t* pGroupMain = NULL;
lv_obj_t* pContMain = NULL;
lv_obj_t* pLblMainAzuDegree = NULL;
const uint16_t nWidthBar = 2;
lv_obj_t* pBarMainAzuMotSpeed = NULL;
static char sLblMainAzuDegree[8] = "  0";  
lv_obj_t* pLblMainElvDegree = NULL;
lv_obj_t* pBarMainElvMotSpeed = NULL;

static char sLblMainElvDegree[8] = "  0";  
lv_obj_t* pBtnMenu = NULL;
// call back for entering menu
static void menu_btn_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED)
    {
        //ESP_LOGD(tag, "tiggered menu button event");
        display_menu();
    }

}

void display_main_screen()
{
    iCurrentMenu = MENU_MAIN;
    if(pContMain == NULL)
    {
        // ESP_LOGI(tag, "display main screen, detected %d POTs", cfg.N_POTs);
        // group
        pGroupMain = lv_group_create();
        register_group(pGroupMain);

        pContMain = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_add_style(pContMain, LV_CONT_PART_MAIN, &lvStyleSolidCont);
        lv_obj_set_size(pContMain, LV_HOR_RES_MAX,LV_VER_RES_MAX);
        lv_obj_set_style_local_bg_color(pContMain, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        /* Degree displays */
        #ifndef LVGL_SIMULATOR
        if(cfg.N_POTs > 0)
        #endif 
        {
            pLblMainAzuDegree = lv_label_create(pContMain, NULL);
            lv_obj_add_style(pLblMainAzuDegree, LV_LABEL_PART_MAIN, &lvStyleTextLarge);
            lv_obj_set_size(pLblMainAzuDegree, (LV_HOR_RES_MAX - nWidthBar) >> 1, LV_VER_RES_MAX - nWidthBar * 2);
            lv_label_set_text_static(pLblMainAzuDegree, sLblMainAzuDegree);
            lv_label_set_align(pLblMainAzuDegree, LV_LABEL_ALIGN_CENTER);
            lv_obj_align(pLblMainAzuDegree, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
            pBarMainAzuMotSpeed = lv_bar_create(pContMain, NULL);
            lv_bar_set_range(pBarMainAzuMotSpeed, -100, 100);
            lv_obj_set_size(pBarMainAzuMotSpeed, LV_HOR_RES_MAX, nWidthBar);
            lv_obj_align(pBarMainAzuMotSpeed, pContMain, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
        }
        #ifndef LVGL_SIMULATOR
        if(cfg.N_POTs > 1)
        #endif
        {
            pLblMainElvDegree = lv_label_create(pContMain, pLblMainAzuDegree);
            lv_obj_add_style(pLblMainElvDegree, LV_LABEL_PART_MAIN, &lvStyleTextLarge);
            lv_obj_set_size(pLblMainElvDegree, (LV_HOR_RES_MAX - nWidthBar) >> 1, LV_VER_RES_MAX - nWidthBar *2);
            lv_label_set_text_static(pLblMainElvDegree, sLblMainElvDegree);
            lv_label_set_align(pLblMainElvDegree, LV_LABEL_ALIGN_CENTER);
            lv_obj_align(pLblMainElvDegree, NULL, LV_ALIGN_IN_LEFT_MID, (LV_HOR_RES_MAX - nWidthBar) / 2, 0);
            pBarMainElvMotSpeed = lv_bar_create(pContMain, NULL);
            lv_bar_set_range(pBarMainElvMotSpeed, -100, 100);
            lv_obj_set_size(pBarMainElvMotSpeed, nWidthBar, LV_HOR_RES_MAX);
            lv_obj_align(pBarMainElvMotSpeed, pContMain, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        }
        // menu btn
        pBtnMenu = lv_btn_create(pContMain, NULL);
        lv_group_add_obj(pGroupMain, pBtnMenu);
        lv_obj_set_size(pBtnMenu, 1, 1);
        lv_obj_set_style_local_bg_opa(pBtnMenu, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_event_cb(pBtnMenu, menu_btn_cb);
    }
}

/* display menu */
void dismiss_menu();
lv_group_t* pGroupMenu = NULL;
lv_obj_t* pContMenu = NULL;
lv_obj_t* pRollerMenu = NULL;
DEFINE_MULTI_LANG_STR_2(MenuItems, "网络设置\nWiFi状态\n以太网状态\n485串口设置\n返回", "Network Settings\nWiFi Status\nEthernet Status\nRS485\nBack");
MenuCb_t arrFnMenuItems[5] = {display_menu_item_network_settings, display_menu_item_WiFi_status, display_menu_item_ethernet_status, NULL, dismiss_menu};
const uint16_t nMenuItems = sizeof(arrFnMenuItems) / sizeof(MenuCb_t);
static void menu_roller_cb(struct _lv_obj_t* obj, lv_event_t event);
static void display_save_config_prompt();
void display_menu()
{
    iCurrentMenu = MENU_SETTINGS;
    if(pContMenu == NULL)
    {
        //ESP_LOGI(tag, "display menu settings");
        pGroupMenu = lv_group_create();
        register_group(pGroupMenu);
        pContMenu = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_add_style(pContMenu, LV_CONT_PART_MAIN, &lvStyleSolidCont);
        lv_obj_set_size(pContMenu, LV_HOR_RES_MAX,LV_VER_RES_MAX);

        
        pRollerMenu = lv_roller_create(pContMenu, NULL);
        lv_obj_add_style(pRollerMenu, LV_ROLLER_PART_BG, &lvStyleSolidCont);
        lv_obj_add_style(pRollerMenu, LV_ROLLER_PART_BG, &lvStyleTextSmall);
        lv_obj_set_size(pRollerMenu, LV_HOR_RES_MAX, LV_VER_RES_MAX);
        lv_roller_set_visible_row_count(pRollerMenu, 2);
        lv_roller_set_align(pRollerMenu, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(pRollerMenu, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
        lv_roller_set_options(pRollerMenu, MLSTR(MenuItems, lang), LV_ROLLER_MODE_NORMAL);
        lv_obj_set_event_cb(pRollerMenu, menu_roller_cb);
        // add button to group
        lv_group_add_obj(pGroupMenu, pRollerMenu);
        // focus on the roller
        lv_group_set_click_focus(pGroupMenu, true);
        lv_group_focus_obj(pRollerMenu);
        lv_group_focus_freeze(pGroupMenu, true);
        lv_group_set_editing(pGroupMenu, true);
        
    }
}

/* menu callback */
static void menu_roller_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED)
    {
        uint16_t iSel = lv_roller_get_selected(obj);
        if((iSel < nMenuItems) && (arrFnMenuItems[iSel] != NULL))
        {
            arrFnMenuItems[iSel](); // call the function
        }
    }
}

// dismiss menu if no config change, or prompt save config
void dismiss_menu()
{
#ifndef LVGL_SIMULATOR
    if(get_if_config_modified())
    {
        display_save_config_prompt();
    }
    else
#endif
    {
        //if (event == LV_EVENT_RELEASED)
        {
            if(pContMenu)
            {
                // bool e = lv_group_get_editing(pGroupMenu);
                lv_obj_del(pContMenu);
                deregister_group(pGroupMenu);
                pContMenu = NULL;
            }
            iCurrentMenu = MENU_MAIN;
        }

    }
}

/******** prompts BEGIN *****************/
lv_group_t* pGroupSaveConfig = NULL;
lv_obj_t* pMsgBoxSaveConfig = NULL;
DEFINE_MULTI_LANG_STR_2(STR_SAVE_CONFIG, "是否保存更改？", "Save before exit?");
static void save_config_prompt_cb(lv_obj_t* pMsgBox, lv_event_t e);
static void display_save_config_prompt()
{
    static char* btns[] = {"Yes", "No", ""};
    if(pMsgBoxSaveConfig == NULL)
    {
        pGroupSaveConfig = lv_group_create();
        register_group(pGroupSaveConfig);
        pMsgBoxSaveConfig = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(pMsgBoxSaveConfig, MLSTR(STR_SAVE_CONFIG, lang));
        lv_msgbox_add_btns(pMsgBoxSaveConfig, btns);
        lv_obj_align_origo(pMsgBoxSaveConfig, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_event_cb(pMsgBoxSaveConfig, save_config_prompt_cb);
        lv_group_add_obj(pGroupSaveConfig, pMsgBoxSaveConfig);
        lv_group_set_editing(pGroupSaveConfig, true);
    }
}

static void save_config_prompt_cb(lv_obj_t* pMsgBox, lv_event_t e)
{
    if(e == LV_EVENT_RELEASED)
    {
        uint16_t idxBtn = lv_msgbox_get_active_btn(pMsgBoxSaveConfig);
        switch(idxBtn)
        {
            #ifndef LVGL_SIMULATOR
            case 0U: // YES
            // save the config
            // xEventGroupSetBits(evtgrpMain , SIGNAL_SAVE_CONFIG);
            // sync values in config object with rotsensor object
            push_config_to_volatile_variables(&cfg);
            write_to_EEPROM_async();
            // save_config(&cfg);
            // break;
            #endif
            case 1U: // NO
            // dismiss the msgbox
            if(pMsgBoxSaveConfig)
            {
                lv_obj_del(pMsgBoxSaveConfig);
                pMsgBoxSaveConfig = NULL;
                deregister_group(pGroupSaveConfig);
                if (pGroupMenu)
                    lv_group_set_editing(pGroupMenu, true);
            }
            break;
            default: // UNSET
            break;
        }

    }
}
/******** prompts END *******************/

/******     menu items  *********/
/* menu item: network settings */
lv_group_t* pGroupNetworkSettings = NULL;
lv_obj_t* pPageNetworkSettings = NULL;
DEFINE_MULTI_LANG_STR_2(STR_USE_ETHERNET, "使用以太网", "Use Ethernet");
DEFINE_MULTI_LANG_STR_2(STR_USE_WIFI, "使用Wi-Fi", "Use Wi-Fi");
static void lv_group_page_scrolling_cb(lv_group_t* group);
static void dismiss_menu_item_network_settings(lv_obj_t * obj, lv_event_t event);
static void check_use_ethernet_cb(struct _lv_obj_t* obj, lv_event_t event);
static void check_use_wifi_cb(struct _lv_obj_t* obj, lv_event_t event);
void display_menu_item_network_settings()
{
    if(pPageNetworkSettings == NULL)
    {
        iCurrentMenu = MENU_NETWORK;
        pGroupNetworkSettings = lv_group_create();
        register_group(pGroupNetworkSettings);
        lv_group_set_focus_cb(pGroupNetworkSettings, lv_group_page_scrolling_cb);
        //pPageNetworkSettings = lv_scr_act();
        pPageNetworkSettings = lv_page_create(lv_scr_act(), NULL);
        //lv_group_add_obj(pGroupNetworkSettings, pPageNetworkSettings);
        /* Create Checkboxes */
        // use Ethernet
        lv_obj_t* pCkUseEthernet = lv_checkbox_create(pPageNetworkSettings, NULL);
        lv_checkbox_set_text(pCkUseEthernet, MLSTR(STR_USE_ETHERNET, lang));
        lv_obj_align(pCkUseEthernet, pPageNetworkSettings, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        lv_obj_set_event_cb(pCkUseEthernet, check_use_ethernet_cb);
        lv_group_add_obj(pGroupNetworkSettings, pCkUseEthernet);
#ifndef LVGL_SIMULATOR
        lv_checkbox_set_checked(pCkUseEthernet, cfg.use_Ethernet == 1);
#endif
        //lv_page_glue_obj(pCkUseEthernet, true);
        // use WiFi
        lv_obj_t* pCkUseWiFi = lv_checkbox_create(pPageNetworkSettings, pCkUseEthernet);
        lv_checkbox_set_text(pCkUseWiFi, MLSTR(STR_USE_WIFI, lang));
        lv_obj_set_event_cb(pCkUseWiFi, check_use_wifi_cb);
        lv_obj_align(pCkUseWiFi, pCkUseEthernet, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_group_add_obj(pGroupNetworkSettings, pCkUseWiFi);
#ifndef LVGL_SIMULATOR
        lv_checkbox_set_checked(pCkUseWiFi, cfg.use_WiFi == 1);
#endif
        //lv_page_glue_obj(pCkUseWiFi, true);
        /* back button */
        lv_obj_t* pBtnBack = lv_btn_create(pPageNetworkSettings, NULL);
        lv_obj_align(pBtnBack, pCkUseWiFi, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_group_add_obj(pGroupNetworkSettings, pBtnBack);
        //lv_page_glue_obj(pBtnBack, true);
        lv_obj_set_event_cb(pBtnBack, dismiss_menu_item_network_settings);
        lv_obj_t* pLblBack = lv_label_create(pBtnBack, NULL);
        lv_label_set_text(pLblBack, MLSTR(STR_BACK, lang));
        
        //lv_group_set_editing(pGroupNetworkSettings, false);
        
    }
}

static void lv_group_page_scrolling_cb(lv_group_t* group)
{
    lv_obj_t* f = lv_group_get_focused(group);

    /*Get the page content object */
    lv_obj_t* par = lv_obj_get_parent(f);  /*The content object is page so first get scrollable object*/
    if (par) par = lv_obj_get_parent(par);    /*Then get the page itself*/

    /*If the focused object is on the window then scrol lteh window to make it visible*/
    //if (par == lv_win_get_content(win)) 
    {
        //lv_win_focus(win, f, 200);
        lv_page_focus(par, f, LV_ANIM_ON);
    }
}

static void dismiss_menu_item_network_settings(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED)
    {
        if(pPageNetworkSettings )
        {
            iCurrentMenu = MENU_SETTINGS;
            lv_obj_del(pPageNetworkSettings); 
            pPageNetworkSettings = NULL;
            deregister_group(pGroupNetworkSettings);
            // let the roller on the main screen to always be focused when dismiss sub-menus
            if (pGroupMenu)
                lv_group_set_editing(pGroupMenu, true);
        }
        
    }
}

static void check_use_ethernet_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
    #ifndef LVGL_SIMULATOR
        static uint8_t v = 0;
        if(lv_checkbox_is_checked(obj))
            v = 1;
        else
            v = 0;
        set_config_variable_by_name("use_Ethernet", &v);
    #endif
    }
}

static void check_use_wifi_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
    #ifndef LVGL_SIMULATOR
        static uint8_t v = 0;
        if(lv_checkbox_is_checked(obj))
            v = 1;
        else
            v = 0;
        set_config_variable_by_name("use_WiFi", &v);
    #endif
    }
}

static void check_use_rs485_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
    #ifndef LVGL_SIMULATOR
        if(lv_checkbox_get_state(obj) == LV_STATE_CHECKED)
            cfg.use_RS485 = 1;
        else
            cfg.use_RS485 = 0;
    #endif
    }
}


/* menu item WiFi connection status */
#ifdef LVGL_SIMULATOR
typedef enum {
    WIFI_NOT_CONNECTED = 0,
    WIFI_CONN_TYPE_AP,
    WIFI_CONN_TYPE_STA 
} WiFiConnType_t;
WiFiConnType_t WiFiConnType = WIFI_CONN_TYPE_STA;
char sSSIDConn[32] = "TestSSID";
esp_ip_addr_t ipWiFi; // type: IPv4 or IPv6
bool isEthernetConn = true;
esp_ip_addr_t ipEthernet; // type: IPv4 or IPv6
#endif
// TODO: how to exit the item and back to the main menu
lv_obj_t *pPageMenuItemConnection = NULL;
lv_group_t *pGroupMenuItemConnection = NULL;
DEFINE_MULTI_LANG_STR_2(sWiFi, "WiFi", "WiFi");
DEFINE_MULTI_LANG_STR_2(sWiFiNotConn, "未连接", "Not connected");
DEFINE_MULTI_LANG_STR_2(sWiFiAP, "广播", "AP");
DEFINE_MULTI_LANG_STR_2(sWiFiSTA, "连接至", "STA");
static void dismiss_menu_item_WiFi_status(struct _lv_obj_t* obj, lv_event_t event);
void display_menu_item_WiFi_status()
{
    if(pPageMenuItemConnection == NULL)
    {
        pGroupMenuItemConnection = lv_group_create();
        register_group(pGroupMenuItemConnection);
        pPageMenuItemConnection = lv_page_create(lv_scr_act(), NULL);
        /* WiFi status */
        lv_obj_t* pLblWiFi = lv_label_create(pPageMenuItemConnection, NULL);
        lv_label_set_text(pLblWiFi, MLSTR(sWiFi, lang));
        lv_group_add_obj(pGroupMenuItemConnection, pPageMenuItemConnection);
        lv_group_set_editing(pGroupMenuItemConnection, true);
        // TODO: display WiFI status: 
        // AP VORTEX3
        // STA  SSID
        lv_obj_t* pLblWiFiConnTypeAndSSID = lv_label_create(pPageMenuItemConnection, pLblWiFi);
        //lv_group_add_obj(pGroupMenuItemConnection, pLblWiFiConnTypeAndSSID);
        lv_obj_t* pLblWiFiIP = lv_label_create(pPageMenuItemConnection, pLblWiFi);
        //lv_group_add_obj(pGroupMenuItemConnection, pLblWiFiIP);
        char buf[64]={0}, bufIP[64]={0};
        switch(WiFiConnType)
        {
            case WIFI_NOT_CONNECTED:
            strcpy(buf, MLSTR(sWiFiNotConn, lang));
            break;
            case WIFI_CONN_TYPE_AP:
            sprintf(buf, "%s %s", MLSTR(sWiFiAP, lang), sSSIDConn);
            buf[sizeof(buf) - 1] = 0;
            if(ipWiFi.type == ESP_IPADDR_TYPE_V6)
            {
                sprintf(bufIP, IPV6STR, IPV62STR(ipWiFi.u_addr.ip6));
            }
            else
            {
                sprintf(bufIP, IPSTR, IP2STR(&(ipWiFi.u_addr.ip4)));
            }
            bufIP[sizeof(bufIP) - 1] = 0;
            break;
            case WIFI_CONN_TYPE_STA:
            sprintf(buf, "%s %s", MLSTR(sWiFiSTA, lang), sSSIDConn);
            buf[sizeof(buf) - 1] = 0;
            if(ipWiFi.type == ESP_IPADDR_TYPE_V6)
            {
                sprintf(bufIP, IPV6STR, IPV62STR(ipWiFi.u_addr.ip6));
            }
            else
            {
                sprintf(bufIP, IPSTR, IP2STR(&(ipWiFi.u_addr.ip4)));
            }
            break;
        }
        lv_label_set_text(pLblWiFiConnTypeAndSSID, buf);
        lv_obj_align(pLblWiFiConnTypeAndSSID, pLblWiFi, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,0);
        lv_label_set_text(pLblWiFiIP, bufIP);
        lv_obj_align(pLblWiFiIP, pLblWiFiConnTypeAndSSID, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,0);
        lv_obj_set_event_cb(pPageMenuItemConnection, dismiss_menu_item_WiFi_status);
    }
}

static void dismiss_menu_item_WiFi_status(struct _lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED)
    {

        if(pPageMenuItemConnection)
        {
            lv_obj_del(pPageMenuItemConnection);
            deregister_group(pGroupMenuItemConnection);
            pPageMenuItemConnection = NULL;
            if (pGroupMenu)
                lv_group_set_editing(pGroupMenu, true);
        }
    }
}


/* menu item Ethernet connection status */
lv_obj_t *pPageEthernetStatus = NULL;
lv_group_t *pGroupEthernetStatus = NULL;
DEFINE_MULTI_LANG_STR_2(sEthernet, "以太网", "Ethernet");
DEFINE_MULTI_LANG_STR_2(sEthernetNotConn, "未连接", "Not connected");
DEFINE_MULTI_LANG_STR_2(sEthernetConn, "已连接", "Connected");
static void dismiss_menu_item_Ethernet_status(struct _lv_obj_t* obj, lv_event_t event);
void display_menu_item_ethernet_status()
{
    if(pPageEthernetStatus == NULL)
    {
        pGroupEthernetStatus = lv_group_create();
        register_group(pGroupEthernetStatus);

        pPageEthernetStatus = lv_page_create(lv_scr_act(), NULL);
        lv_obj_set_width(pPageEthernetStatus, LV_HOR_RES_MAX);
        lv_obj_set_event_cb(pPageEthernetStatus, dismiss_menu_item_Ethernet_status);
        lv_page_set_scrlbar_mode(pPageEthernetStatus, LV_SCRLBAR_MODE_AUTO);
        lv_group_add_obj(pGroupEthernetStatus, pPageEthernetStatus);
        lv_group_set_editing(pGroupEthernetStatus, true);
        /* Ethernet status */
        lv_obj_t* pLblEthernet = lv_label_create(pPageEthernetStatus, NULL);
        lv_label_set_text(pLblEthernet, MLSTR(sEthernet, lang));
        //lv_group_add_obj(pGroupEthernetStatus, pLblEthernet);
        // TODO: display WiFI status: 
        // IP
        char bufIP[64] = {0};
        lv_obj_t* pLblEthernetIP = lv_label_create(pPageEthernetStatus, pLblEthernet);
        //lv_group_add_obj(pGroupEthernetStatus, pLblEthernetIP);
        if(isEthernetConn)
        {
            if(ipEthernet.type == ESP_IPADDR_TYPE_V6)
            {
                sprintf(bufIP, IPV6STR, IPV62STR(ipEthernet.u_addr.ip6));
            }
            else
            {
                sprintf(bufIP, IPSTR, IP2STR(&(ipEthernet.u_addr.ip4)));
            }
        }
        else
        {
            strcpy(bufIP, MLSTR(sEthernetNotConn, lang));
        }
        lv_label_set_text(pLblEthernetIP, bufIP);
        lv_obj_align(pLblEthernetIP, pLblEthernet, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    }
}

static void dismiss_menu_item_Ethernet_status(struct _lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED)
    {
        if(pPageEthernetStatus)
        {
            lv_obj_del(pPageEthernetStatus);
            deregister_group(pGroupEthernetStatus);
            pPageEthernetStatus = NULL;
            if (pGroupMenu)
                lv_group_set_editing(pGroupMenu, true);
        }
    }
}


// Display value updaters
// call this function to update value
void display_update_azu_degree(float degree)
{
    // invalid value, display "---"
    if(degree == (float)LOOP_BUFFER_INVALID_VALUE)
    {
        #ifndef LVGL_SIMULATOR
        xSemaphoreTake(mtxDisplay, portMAX_DELAY);
        #endif
        snprintf(sLblMainAzuDegree, sizeof(sLblMainAzuDegree), "---");
        sLblMainAzuDegree[sizeof(sLblMainAzuDegree)-1] = 0;
        if (pLblMainAzuDegree)
            lv_obj_invalidate(pLblMainAzuDegree); // force to redraw the label
        #ifndef LVGL_SIMULATOR
        xSemaphoreGive(mtxDisplay);
        #endif
    }
    // valid value
    else
    {

        while(degree < 0.f)
            degree += 360.f;
        while(degree > 360.f)
            degree -= 360.f;
        #ifndef LVGL_SIMULATOR
        xSemaphoreTake(mtxDisplay, portMAX_DELAY);
        #endif
        snprintf(sLblMainAzuDegree, sizeof(sLblMainAzuDegree), "%3u", (unsigned int)degree);
        sLblMainAzuDegree[sizeof(sLblMainAzuDegree)-1] = 0;
        if (pLblMainAzuDegree)
            lv_obj_invalidate(pLblMainAzuDegree); // force to redraw the label
        #ifndef LVGL_SIMULATOR
        xSemaphoreGive(mtxDisplay);
        #endif
    }
}



// call this function to update value
void display_update_elv_degree(float degree)
{
    // invalid value, display "---"
    if(degree == (float)LOOP_BUFFER_INVALID_VALUE)
    {
        #ifndef LVGL_SIMULATOR
        xSemaphoreTake(mtxDisplay, portMAX_DELAY);
        #endif
        snprintf(sLblMainElvDegree, sizeof(sLblMainElvDegree), "---");
        sLblMainElvDegree[sizeof(sLblMainElvDegree)-1] = 0;
        if (pLblMainElvDegree)
            lv_obj_invalidate(pLblMainElvDegree);
        #ifndef LVGL_SIMULATOR
        xSemaphoreGive(mtxDisplay);
        #endif
    }
    else
    {
        while(degree < 0.f)
            degree += 360.f;
        while(degree > 360.f)
            degree -= 360.f;
        #ifndef LVGL_SIMULATOR
        xSemaphoreTake(mtxDisplay, portMAX_DELAY);
        #endif
        snprintf(sLblMainElvDegree, sizeof(sLblMainElvDegree), "%3u", (unsigned int)degree);
        sLblMainElvDegree[sizeof(sLblMainElvDegree)-1] = 0;
        if (pLblMainElvDegree)
            lv_obj_invalidate(pLblMainElvDegree);
        #ifndef LVGL_SIMULATOR
        xSemaphoreGive(mtxDisplay);
        #endif
    }
}



void display_update_azu_mot_speed(int16_t speed100)
{
    if(pBarMainAzuMotSpeed)
    {
        lv_bar_set_value(pBarMainAzuMotSpeed, speed100, LV_ANIM_OFF );
        // lv_obj_invalidate(pBarMainAzuMotSpeed);
    }    
}

void display_update_elv_mot_speed(int16_t speed100)
{
    if(pBarMainElvMotSpeed)
    {
        lv_bar_set_value(pBarMainElvMotSpeed, speed100, LV_ANIM_OFF);
        // lv_obj_invalidate(pBarMainElvMotSpeed);
    }    
}
