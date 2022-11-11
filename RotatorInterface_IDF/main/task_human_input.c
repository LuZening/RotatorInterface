#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lvgl.h"
#include "ButtonDebouncer.h"
#include "driver/adc.h"
#include "main.h"
#include "RotTask.h"
#include "esp_log.h"
#include "task_display.h"
#define tag "task_human_input"

// stack size = 2048 + 256

const int ADC_MANUAL_BITWIDTH = 12UL;
const int ADC_MANUAL_MAXVALUE = ((1UL << ADC_MANUAL_BITWIDTH) - 1UL);
const int ADC_MANUAL_OVERSAMPLING = 4UL;
// ADC1.3
const adc_unit_t ADC_UNIT_MANUAL = ADC_UNIT_1;
const adc_channel_t ADC_CHANNEL_MANUAL = ADC_CHANNEL_3;
// buttons 
#define N_BUTTONS 3
ButtonDebouncer_t btns[N_BUTTONS];
const int BUTTON_ADC_VALUE_TOLERANCE = ADC_MANUAL_MAXVALUE / 20;
const int BUTTON_ADC_VALUE_THRESHOLDS[] = { ADC_MANUAL_MAXVALUE / 12 * 3,  // SET
                                ADC_MANUAL_MAXVALUE / 12 * 7,  // CCW
                                ADC_MANUAL_MAXVALUE / 12 * 9.5, // CW 
                                };
// the currently pressed button id, 0,1,2 ... N_BUTTONS-1, < means not pressed
int iBtnPressed = -1; // = 0: no button pressed 1 : SET 2: CCW 3: CW
// maps pressed button ids to button actions
HumanInputType_t BTN_ID_TO_ACTIONS[N_BUTTONS] = {
    BUTTON_SET, BUTTON_CW, BUTTON_CCW
};
// maps pressed button ids to LVGL key actions
lv_key_t LV_KEY_ACTIONS[N_BUTTONS] = {
    LV_KEY_ENTER, LV_KEY_RIGHT, LV_KEY_LEFT
};

// timing
const int TICK_PERIOD_US = 10000;
const int MANUAL_TASK_MAX_DURATION_MS = 60000;
/* OS objects */
// output processed human inputs to display tasks
QueueHandle_t qHumanInputOperations;


uint16_t nRotEncCounter;

void init_human_input_task()
{
    qHumanInputOperations = xQueueCreate(8, sizeof(HumanInputType_t));
}

void task_human_input(void* args)
{
    /* setup */
    TickType_t tick;
    int i = 0, vADCRaw = 0;
    
    for(i = 0; i < N_BUTTONS; ++i)
    {
        btndeb_init(&(btns[i]), -1, 40000UL);
    }

    // record the id of the previously pressed button
    int iPrevBtnPressed = -1;
    nRotEncCounter = 0;
    // record the previously sent task
    RotTask_t rotTaskPrev;
    init_task(&rotTaskPrev);
    rotTaskPrev.motnum = iCurrentSelectedMotor; // defined in task_display.c
    ESP_LOGD(tag, "task initialized");
    /* super loop */
    tick = xTaskGetTickCount();
    while(1)
    {
        /* Oversampling */
        for(i = 0; i < ADC_MANUAL_OVERSAMPLING; ++i)
        {
            // ADC bit widths set to 12bit by default
            vADCRaw += adc1_get_raw(ADC_CHANNEL_MANUAL);
        }
        vADCRaw = vADCRaw / ADC_MANUAL_OVERSAMPLING;
        int nBtnRetVal = 0; // button tick return value, 0: not flipped 1: flipped
        bool hasBtnFlipped = false; // true if any of the buttons is flipped
        bool isButtonMatched = false; 
        /* decipher ADC values, and translate it to the button pressed */
        for(i = 0; i < N_BUTTONS; ++i)
        {
            // button pressed
            if((!isButtonMatched) &&  (vADCRaw < BUTTON_ADC_VALUE_THRESHOLDS[i] + BUTTON_ADC_VALUE_TOLERANCE))
            {
                nBtnRetVal = btndeb_tick(&btns[i], TICK_PERIOD_US, 0);
                isButtonMatched = true;
            }
            // button not pressed, 
            else
            {
                nBtnRetVal = btndeb_tick(&btns[i], TICK_PERIOD_US, 1);
            }
            // if button state flipped, execute the corresponding routine
            if(nBtnRetVal > 0)
            {
                hasBtnFlipped = true;
                // pressed down
                if(btns[i].state == 0)
                {
                    /* Button Pressed, do something BEGIN */
                    // xQueueSendToBack(qHumanInputOperations, &BUTTON_ACTIONS[i], 1);
                    iPrevBtnPressed = iBtnPressed;
                    iBtnPressed = i;
                    ESP_LOGD(tag, "button %d pressed, ADC = %d", i, vADCRaw);
                    /* Button Pressed, do something END */
                }
                // released
                else
                {
                    if(iBtnPressed >= 0)
                    {
                        switch(LV_KEY_ACTIONS[iBtnPressed])
                        {
                            case LV_KEY_RIGHT:
                            nRotEncCounter++;
                            break;
                            case LV_KEY_LEFT:
                            nRotEncCounter--;
                            break;
                            default:
                            break;
                        }
                    }
                    iPrevBtnPressed = iBtnPressed;
                    iBtnPressed = -1;
                    ESP_LOGD(tag, "button %d released, ADC = %d", i, vADCRaw);
                }
            }
        }
        /* When display is at the main screen, There is a button pressed/released, do something BEGIN*/
        if(iCurrentMenu == MENU_MAIN)
        {
            if(hasBtnFlipped)
            {
                RotTask_t rotTaskNew;
                init_task(&rotTaskNew);
                // has button pressed
                if(iBtnPressed >= 0)
                {
                    // button id being pressed has changed without going through a release, 
                    // force the previously pressed button to be released
                    if(iPrevBtnPressed >= 0 && iPrevBtnPressed != iBtnPressed)
                    {
                        if(rotTaskPrev.type != ROT_TASK_NULL)
                        {
                            // send a stop task first
                            rotTaskNew.type = ROT_TASK_NULL;
                            rotTaskNew.motnum = rotTaskPrev.motnum;
                            // xQueueSendToBack(qRotTasks, &rotTaskNew, 0);
                        }
                    }
                    // send task according to button id
                    switch(BTN_ID_TO_ACTIONS[iBtnPressed])
                    {
                        case BUTTON_CW:
                        // create a CW MANUAL task
                        rotTaskNew.type = ROT_TASK_MANUAL;
                        rotTaskNew.motnum = iCurrentSelectedMotor + 1; // defined in task_display.c
                        rotTaskNew.n_to = 90000;
                        rotTaskNew.n_speed = cfg.motSpeeds[rotTaskNew.motnum - 1];
                        xQueueSendToBack(qRotTasks, &rotTaskNew, 0);
                        break;
                        case BUTTON_CCW:
                        // create a CCW MANUAL task
                        rotTaskNew.type = ROT_TASK_MANUAL;
                        rotTaskNew.motnum = iCurrentSelectedMotor + 1; // defined in task_display.c
                        rotTaskNew.n_to = -90000;
                        rotTaskNew.n_speed = cfg.motSpeeds[rotTaskNew.motnum - 1];
                        xQueueSendToBack(qRotTasks, &rotTaskNew, 0);
                        break;
                        // if none of the above, send a null task
                        case BUTTON_SET:
                        // show task list status
                        // print_task_status();
                        break;
                        default:
                        rotTaskNew.type = ROT_TASK_NULL;
                        rotTaskNew.motnum = iCurrentSelectedMotor + 1; // defined in task_display.c
                        // xQueueSendToBack(qRotTasks, &rotTaskNew, 0);
                        break;
                    }
                    rotTaskPrev = rotTaskNew;
                }
                else // buttons are released
                {
                    if(rotTaskPrev.type != ROT_TASK_NULL)
                    {
                        // send a stop task
                        rotTaskNew.type = ROT_TASK_NULL;
                        rotTaskNew.motnum = iCurrentSelectedMotor + 1; // defined in task_display.c
                        xQueueSendToBack(qRotTasks, &rotTaskNew, 0);
                        rotTaskPrev = rotTaskNew;
                    }
                }
            }
        }
        /* There is a button pressed/released, do something END*/
        // vTaskDelayUntil(&tick, pdMS_TO_TICKS(TICK_PERIOD_US / 1000));
        vTaskDelay(pdMS_TO_TICKS(TICK_PERIOD_US / 1000));
    }
}

// Callback function for LVGL to read button actions, type BUTTONS
bool lv_read_cb_rot_enc(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint16_t nRotEncCounterOld = 0;
    uint16_t encdiff  = nRotEncCounter - nRotEncCounterOld;
    bool pressedEnter = (iBtnPressed >= 0 && LV_KEY_ACTIONS[iBtnPressed] == LV_KEY_ENTER);
    nRotEncCounterOld = nRotEncCounter;
    if(encdiff != 0 || pressedEnter)
    {
        if(isDisplayOn)
        {
            if(pressedEnter)
                data->state = LV_INDEV_STATE_PR;
            else
                data->state = LV_INDEV_STATE_REL;
            data->enc_diff = encdiff;
        }
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
        data->enc_diff = 0;
    }
    if(iBtnPressed >= 0)
    {
        display_turn_on();
    }
    return false;
}