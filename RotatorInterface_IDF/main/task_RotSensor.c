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
#include "webserver.h"


#define N_ROT_SENSORS 2
#define ROTSENSOR_SAMPLE_INTERVAL_US 20000UL

SemaphoreHandle_t mutexRotSensor = NULL;
RotSensor arrRotSensors[N_ROT_SENSORS];
int arrDegreeValues[N_ROT_SENSORS] = {-255};
int arrRotADCRawValues[N_ROT_SENSORS] = {0};
int arrObservedMotSpeeds[N_ROT_SENSORS] = {0}; // deg / sec
int arrLimits[N_ROT_SENSORS] = {0};

/* ADC hardware config BEGIN */
// Define ADC Units
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


/* ADC hardware config END */
static void init_rot_sensor_ADC()
{
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
}



int get_current_degree_value(int i)
{
    return arrDegreeValues[i];
}


void task_rot_sensor(void* args)
{
    char sensorData[256] = {0};
    int lenSensorData = 0;
    /* setup */
    init_rot_sensor_ADC();
    const int interval_us = 20000;
    /* Config RotSensors */
    for(int i = 0; i < cfg.N_POTs; ++i)
    {
        RotSensor* pRot = &(arrRotSensors[i]);
        RotSensor_init(pRot, cfg.pot_type[i],  cfg.allow_multi_rounds[i], cfg.brake_engage_defer[i]);
        RotSensor_set_resistors(pRot, cfg.R_max[i], cfg.R_min[i], cfg.R_0[i], cfg.R_c[i]);
        RotSensor_set_ADC_range(pRot, cfg.ADC_min[i], cfg.ADC_max[i], cfg.ADC_zero[i], cfg.deg_min[i], cfg.deg_max[i]);
    }
    uint32_t counter = 0;
    while(1)
    {
        bool areAllSensorsWellFilled = true;
        counter++;
        for(int i = 0; i < cfg.N_POTs; ++i)
        {
            int vADCRaw = 0;
            RotSensor* pRot = &(arrRotSensors[i]);
            adc_unit_t adcUnit = ADC_UNITS[i];
            adc_channel_t adcCh = ADC_CHANNELS[i];
            /* Oversample the ADC 16 times and take the average  */
            for(int j = 0; j < 16; ++j)
            {
                if(adcUnit == ADC_UNIT_1)
                {
                    vADCRaw += adc1_get_raw(adcCh);
                }   
                else if(adcUnit == ADC_UNIT_2)
                {
                    int vADCRaw_new;
                    adc2_get_raw(adcCh, ADC_WIDTH, &vADCRaw_new );
                    vADCRaw += vADCRaw_new;
                }
            }
            vADCRaw >>= 4; // vADCRaw is the average sampled value

            // Convert raw value to pot position
            xSemaphoreTake(mutexRotSensor, 0);
            RotSensor_append_ADC_reading(pRot, vADCRaw);
            arrDegreeValues[i] = RotSensor_get_degree(pRot);
            arrRotADCRawValues[i] = RotSensor_get_ADC(pRot);
            xSemaphoreGive(mutexRotSensor);
            if(!LoopBuffer_is_full(&pRot->deg_results))
                areAllSensorsWellFilled = false;
        }
        /* Broadcast sensor data over Websocket  */
        // each 10 cycles 10*20ms = 200ms
        if(counter % 10 == 0)
        {
            // only send sensor data when sensor buffer is well filled
            if(areAllSensorsWellFilled)
            {
                lenSensorData = get_sensor_data_string(sensorData, sizeof(sensorData));
                websocket_broadcast("/ws", sensorData, lenSensorData);
            }
        }
        // Use task delay to control sample rate 20ms
        vTaskDelay(pdMS_TO_TICKS((ROTSENSOR_SAMPLE_INTERVAL_US / 1000UL))); 
    }
}


// num=%d&ADC=%d&deg=%d&busy=%d&limit=%d&speed=%d
// num = 1 to N_SENSORS
int get_sensor_data_string(char* buf, int lenbuf)
{
    // sprintf(s_httpbuffer, PSTR("azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"),
    int nWritten = 0;
    xSemaphoreTake(mutexRotSensor, 0);
    for(int i = 0; i < cfg.N_POTs; ++i)
    {
        if(nWritten >= lenbuf - 1)
            break;
        int n = snprintf(buf + nWritten, lenbuf - nWritten - 1, "num=%d&ADC=%d&deg=%d&motstatus=%d&limit=%d&speed=%d\n", 
                i+1,
                arrRotADCRawValues[i],
                arrDegreeValues[i],
                (arrMotors[i].status),
                (arrLimits[i]),
                (arrObservedMotSpeeds[i]));
        nWritten += n;
    }
    xSemaphoreGive(mutexRotSensor);
    buf[lenbuf-1] = 0; // terminate the string
    return nWritten;
}