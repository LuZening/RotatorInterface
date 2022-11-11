/*

Task for reading rot sensor data from ADC

*/

#include "RotSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "Config.h"
#include "main.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "webserver.h"
#include "stdint.h"
#include "esp_log.h"
#include "driver/adc.h"
#define tag "task_rot_sensor"

#define N_ROT_SENSORS 2
#define ROTSENSOR_SAMPLE_INTERVAL_US 10000UL


SemaphoreHandle_t mutexRotSensor;
RotSensor arrRotSensors[N_ROT_SENSORS];
int arrDegreeValues[N_ROT_SENSORS] = {LOOP_BUFFER_INVALID_VALUE};
int arrRotADCRawValues[N_ROT_SENSORS] = {LOOP_BUFFER_INVALID_VALUE};
int arrObservedMotSpeeds[N_ROT_SENSORS] = {0}; // deg / sec
int arrLimits[N_ROT_SENSORS] = {0};

/* ADC hardware config BEGIN */
// Define ADC Units for motors
const adc_unit_t ADC_UNITS[N_ROT_SENSORS] = {
    ADC_UNIT_1, // ADC_UNIT_1 covers GPIO32-39
    ADC_UNIT_1
};
// Define ADC width
const adc_bits_width_t ADC_WIDTH = ADC_WIDTH_12Bit;
const uint32_t ADC_MAX_VAL = ((1L << 12) - 1);
// Define ADC channels
const adc_channel_t ADC_CHANNELS[N_ROT_SENSORS] = {
    ADC_CHANNEL_6,  // GPIO34 POT1
    ADC_CHANNEL_7}; // GPIO35 POT2
// Define ADC attenuation
const adc_atten_t ADC_ATTENS[N_ROT_SENSORS] = {
    ADC_ATTEN_DB_2_5, // -2.5dB attenuation, input range spans to 1.25V
    ADC_ATTEN_DB_2_5
};

// Define ADC unit for main current sensor
const adc_unit_t ADC_UNIT_IMAIN = ADC_UNIT_1;
const adc_channel_t ADC_CHANNEL_IMAIN = ADC_CHANNEL_0; 

/* ADC hardware config END */
esp_adc_cal_characteristics_t adcChars1;
void init_rot_sensor_task()
{
    mutexRotSensor = xSemaphoreCreateMutex();
    // config pot ADC
    for(int i = 0; i < N_ROT_SENSORS; ++i)
    {
        if(ADC_UNITS[i] == ADC_UNIT_1)
        {
            adc1_config_width(ADC_WIDTH);
            adc1_config_channel_atten(ADC_CHANNELS[i], ADC_ATTENS[i]);
        }
        else
        {
            adc2_config_channel_atten(ADC_CHANNELS[i], ADC_ATTENS[i]);
        }

    }
    // config Imain ADC
    if(ADC_UNIT_IMAIN == ADC_UNIT_1)
    {
        // from 2.5V ~ 3.9V
        adc1_config_channel_atten(ADC_CHANNEL_IMAIN, ADC_ATTEN_DB_11);

    }
    /* calibrate ADC */
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTENS[0], ADC_WIDTH, 1100, &adcChars1);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGD(tag, "ADC has eFuse Vref data");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGW(tag, "ADC has Two Point calibration data");
    } else {
        ESP_LOGW(tag, "ADC has no calibration data");
    }
    /* Config RotSensors */
    for(int i = 0; i < cfg.N_POTs; ++i)
    {
        RotSensor* pRot = &(arrRotSensors[i]);
        RotSensor_init(pRot, cfg.pot_type[i],  cfg.allow_multi_rounds[i], cfg.brake_engage_defer[i]);
        RotSensor_set_resistors(pRot, cfg.R_max[i], cfg.R_min[i], cfg.R_0[i], cfg.R_c[i]);
        RotSensor_set_ADC_range(pRot, cfg.ADC_min[i], cfg.ADC_max[i], cfg.ADC_zero[i], cfg.deg_min[i], cfg.deg_max[i]);
    }
}


int get_current_degree_value(int i)
{
    return arrDegreeValues[i];
}


void task_rot_sensor(void* args)
{
    TickType_t tick, tickBegin;
    char sensorData[256] = {0};
    int lenSensorData = 0;
    /* setup */
    tick = xTaskGetTickCount();
    tickBegin = tick;
    ESP_LOGV(tag, "Number of sensors %d", cfg.N_POTs);
    while(1)
    {
        bool areAllSensorsWellFilled = true;
        /* read motor pots */
        for(int i = 0; i < cfg.N_POTs; ++i)
        {
            uint32_t vADCmV= 0, vADCmv_Filtered = 0;
            // uint32_t vADCRaw = 0;
            RotSensor* pRot = &(arrRotSensors[i]);
            adc_unit_t adcUnit = ADC_UNITS[i];
            adc_channel_t adcCh = ADC_CHANNELS[i];
            /* Oversample the ADC 4 times and take the average  */
            for(int j = 0; j < 8; ++j)
            {

                if(adcUnit == ADC_UNIT_1)
                {
                    esp_adc_cal_get_voltage(adcCh, &adcChars1, &vADCmV);
                    vADCmv_Filtered += vADCmV;
                    // vADCRaw += adc1_get_raw(adcCh);

                }   
                else if(adcUnit == ADC_UNIT_2)
                {
                    int vADCRaw_new;
                    adc2_get_raw(adcCh, ADC_WIDTH, &vADCRaw_new );
                    // vADCRaw += vADCRaw_new;
                    vADCmv_Filtered += vADCRaw_new;
                }
            }
            // vADCRaw >>= 5; // vADCRaw is the average sampled value
            vADCmv_Filtered >>= 3;
            // ESP_LOGD(tag, "sensor ADC = %d", vADCRaw);

            // Convert raw value to pot position
            xSemaphoreTake(mutexRotSensor, portMAX_DELAY);
            RotSensor_append_ADC_reading(pRot, vADCmv_Filtered);
            xSemaphoreGive(mutexRotSensor);
            if(!LoopBuffer_is_full(&pRot->deg_results))
            {
                areAllSensorsWellFilled = false;
            }
            else
            {
                arrRotADCRawValues[i] = RotSensor_get_ADC(pRot);
                arrDegreeValues[i] = RotSensor_get_degree(pRot);
            }
        }
        /* Broadcast sensor data over Websocket  */
        // each 100 ticks = 100ms
        if((tick - tickBegin) % 100 == 0)
        {
            // only send sensor data when sensor buffer is well filled
            if(areAllSensorsWellFilled)
            {
                lenSensorData = get_sensor_data_string(sensorData, sizeof(sensorData));
                websocket_broadcast("/ws", sensorData, lenSensorData);
            }
        }
        /* read main current Imain ADC */
        int vADCRaw = 0;
        int Imain_mA = 0;
        for(int j = 0; j < 16; ++j)
        {
            vADCRaw += adc1_get_raw(ADC_CHANNEL_IMAIN);
        }
        vADCRaw >>= 4;
        // convert adc value to Imain mA
        // the chip is bidirectional, need to zero before use
        // Use task delay to control sample rate 20Hz
        // vTaskDelay(pdMS_TO_TICKS((ROTSENSOR_SAMPLE_INTERVAL_US / 1000UL))); 
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(ROTSENSOR_SAMPLE_INTERVAL_US / 1000UL));
    }
}


// num=%d&ADC=%d&deg=%d&busy=%d&limit=%d&speed=%d
// num = 1 to N_SENSORS
int get_sensor_data_string(char* buf, int lenbuf)
{
    // sprintf(s_httpbuffer, PSTR("azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"),
    int nWritten = 0;
    xSemaphoreTake(mutexRotSensor, portMAX_DELAY);
    for(int i = 0; i < cfg.N_POTs; ++i)
    {
        if(nWritten >= lenbuf - 1)
            break;
        int n = snprintf(buf + nWritten, lenbuf - nWritten - 1, "num=%d&ADC=%d&deg=%d&motstatus=%d&dir=%d&limit=%d&speed=%d\n", 
                i+1,
                arrRotADCRawValues[i],
                arrDegreeValues[i],
                (arrMotors[i].motStatus),
                (arrMotors[i].direction),
                (arrLimits[i]),
                (arrObservedMotSpeeds[i]));
        nWritten += n;
    }
    xSemaphoreGive(mutexRotSensor);
    buf[lenbuf-1] = 0; // terminate the string
    return nWritten;
}

