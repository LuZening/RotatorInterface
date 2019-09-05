#include <Arduino.h>
#include "Config.h"
#include "RotSensor.h"
// TODO:
// dADC / dt time-based limit switch
// REVERSE
RotSensor::RotSensor(PotType pot_type, unsigned long update_interval)
{
    this->pot_type = pot_type;
    this->update_interval = update_interval;
    this->updates_per_sec = 1000000L / update_interval;
    allow_multi_rounds = false;
    ADC_reading = 0;
    degree = DEGREE_INVALID;
    ADC_max = ADC_MAX;
    ADC_min = ADC_MIN;
    ADC_per_degree = (ADC_max - ADC_min) / 360.;
    ADC_halfway = (ADC_max + ADC_min) >> 1;
    ADC_Q1 = (ADC_max - ADC_min) * 1 / 4 + ADC_min;
    ADC_T1 = (ADC_max - ADC_min) * 1 / 3 + ADC_min;
    ADC_Q3 = (ADC_max - ADC_min) * 3 / 4 + ADC_min;
    ADC_T2 = (ADC_max - ADC_min) * 2 / 3 + ADC_min;
    inverse_ADC = false;
    ADC_zero = 0;
    idx_data = 0;
    n_data = 0;
    for (int i = 0; i < LEN_FILTER_WINDOW; ++i)
    {
        ADC_data[i] = -255;
    }
    clipper = -255;
    EMA_1 = -255;
    EMA_2 = -255;
    is_stable = true;
    is_ADC_calibrated = false;
    ADC_prev_fil = -255;
    n_rounds = 0;
    n_edge_freeze = 0;
    stat_edge_F = 0;
    stat_edge_R = 0;
    stat_edge_T1 = 0;
    stat_edge_T2 = 0;
#ifndef HARD_LIMIT_SWITCH
    deg_limit_F = 360 + EXT_RANGE;
    deg_limit_B = -EXT_RANGE;
#else
    deg_limit_F = 355;
    deg_limit_B = 5;
#endif
    stat_limit_F = 0;
    stat_limit_B = 0;
    stat_limit_T = 0;
    on_edge = NULL;
    ADC_results = new LoopBuffer(LEN_RESULTS);
    deg_results = new LoopBuffer(LEN_RESULTS);
    n_ADC_speed = 0;
    n_deg_speed = 0;
    R_0 = R0_DEFAULT;
    R_c = Rc_DEFAULT;
    R_max = RMAX_DEFAULT;
    R_min = RMIN_DEFAULT;
}

void RotSensor::update_result(int r)
{
    ADC_results->append(r);
    ADC_reading = r;
    if (true) // is stable
    {
        degree = ADC2deg(r);
        deg_results->append(degree);
        n_deg_speed = deg_results->get_latest() - deg_results->get_oldest();
        n_ADC_speed = ADC_results->get_latest() - ADC_results->get_oldest();
    }
    if (degree == DEGREE_INVALID)
        return;
    if (degree >= deg_limit_F && stat_limit_F < LIMIT_STATE_STAGE) // reached the forward limit
    {
        stat_limit_F++;
    }
    else if (degree <= deg_limit_B && stat_limit_B < LIMIT_STATE_STAGE) // reached the backward limit
    {
        stat_limit_B++;
    }
    else // within the range
    {
        if (stat_limit_F > 0)
            stat_limit_F--;
        if (stat_limit_B > 0)
            stat_limit_B--;
    }
}
void RotSensor::set_ADC_range(int inf, int sup, int zero)
{
    ADC_max = sup;
    ADC_min = inf;
    ADC_zero = zero;
    update_ADC();
    // DEBUG
    Serial.printf("ADC_max=%d,ADC_min=%d,ADC_zero=%d,X_zero=%d,X_sup=%d,X_zero=%d,X_inf=%d,deg_inf=%d,deg_zero=%d,deg_sup=%d\n",
                  sup, inf, zero,
                  (int)(X_zero * 100),
                  (int)(ADC2res_scale(ADC_max) * 100),
                  (int)(ADC2res_scale(ADC_zero) * 100),
                  (int)(ADC2res_scale(ADC_min) * 100),
                  ADC2deg(ADC_min),
                  ADC2deg(ADC_zero),
                  ADC2deg(ADC_max));
}
void RotSensor::update_ADC() // TODO: modify
{
    ADC_halfway = ((ADC_max - ADC_min) >> 2) + ADC_min;
    ADC_per_degree = (ADC_max - ADC_min) / 360.;
    ADC_Q1 = (ADC_max - ADC_min) * 1 / 4 + ADC_min;
    ADC_T1 = (ADC_max - ADC_min) * 1 / 3 + ADC_min;
    ADC_Q3 = (ADC_max - ADC_min) * 3 / 4 + ADC_min;
    ADC_T2 = (ADC_max - ADC_min) * 2 / 3 + ADC_min;
    // calibration for TWO_TERMINAL potentiometer
    // calculate X_max, R_min, X_zero
    ADC_maxmin = (float)ADC_max / ADC_min;
    X_max = (float)R_max / (float)(R_max + R_0);
    R_min = (int)(X_max * R_0 / (ADC_maxmin - X_max) + 0.5);
    X_min = (float)R_min / (float)(R_min + R_0);
    X_zero = ADC2res_scale(ADC_zero);
    Serial.printf("A=%d, X_max=%d, R_min=%d, X_min=%d\n",
                   (int)(ADC_maxmin * 100),
                   (int)(X_max * 100),
                   (int)(R_min * 100),
                   (int)(X_min * 100));
    is_ADC_calibrated = true;
    is_stable = true;
}

// call each 50ms
void RotSensor::append_ADC_reading(int data)
{
    // clipper
    if (clipper < 0) // clipper is not initialized
    {
        clipper = data;
    }
    else
    {
        if (data > clipper + CLIPPER_DEADZONE)
            clipper = clipper + CLIPPER_DEADZONE;
        else if (data < clipper - CLIPPER_DEADZONE)
            clipper = clipper - CLIPPER_DEADZONE;
        else
            clipper = data;
    } // clipping
    //EMA_1
    if (EMA_1 < 0)
        EMA_1 = clipper;
    else
        EMA_1 += (clipper - EMA_1) / EMA_ALPHA_1;
    //EMA_2
    if (EMA_2 < 0)
        EMA_2 = EMA_1;
    else
        EMA_2 += (EMA_1 - EMA_2) / EMA_ALPHA_2;
    // filtered output EMA_2
    // come here each update_interval * DATA_OVERHEAD = 100ms
    if (n_data++ >= DATA_OVERHEAD)
    {
        n_data = 0;
        int d = int(EMA_2);
        update_result(d);
        if (n_edge_freeze > 0) // falling edge freeze
            n_edge_freeze--;
        else if (n_edge_freeze < 0) // rising edge freeze
            n_edge_freeze++;
        // interval deviation edge detection
        if (ADC_results->is_full())
        {
            int n_change = (ADC_results->get_latest() - ADC_results->get_oldest());
            // falling edge state machine
            if (n_edge_freeze <= 0)
            {
                if ((stat_edge_F < LIMIT_STATE_STAGE))
                {
                    if (n_change < -EDGE_THRESHOLD)
                    {
                        stat_edge_F += 1;
                        is_stable = false;
                    }
                    else
                    {
                        is_stable = true;
                    }
                }
                else if (stat_edge_F < 2 * LIMIT_STATE_STAGE)
                {
                    if (d > ADC_Q3)
                    {
                        stat_edge_F = 0;
                        is_stable = true;
                    }
                    else if (d < ADC_Q1)
                    {
                        ++stat_edge_F;
                    }
                }
                else if (stat_edge_F == 2 * LIMIT_STATE_STAGE)
                {
                    on_falling_edge();
                    is_stable = true;
                    stat_edge_F = 0;
                    n_edge_freeze = EDGE_FREEZE_TIME * updates_per_sec / DATA_OVERHEAD;
                }
            }
            // rising edge state machine
            if (n_edge_freeze >= 0)
            {
                if (stat_edge_R < LIMIT_STATE_STAGE)
                {
                    if (n_change > EDGE_THRESHOLD)
                    {
                        ++stat_edge_R;
                        is_stable = false;
                    }
                    else
                    {
                        is_stable = true;
                    }
                }
                else if (stat_edge_R < 2 * LIMIT_STATE_STAGE)
                {
                    if (d < ADC_Q1)
                    {
                        stat_edge_R = 0;
                        is_stable = false;
                    }
                    else if (d > ADC_Q3)
                    {
                        ++stat_edge_R;
                    }
                }
                else if (stat_edge_R == 2 * LIMIT_STATE_STAGE)
                {
                    on_rising_edge();
                    is_stable = true;
                    stat_edge_R = 0;
                    n_edge_freeze = -EDGE_FREEZE_TIME * updates_per_sec / DATA_OVERHEAD;
                }
            }
        }
    }
}

int RotSensor::get_degree()
{
    return degree;
}

int RotSensor::get_ADC()
{
    return ADC_results->get_latest();
}

int RotSensor::is_limit()
{
    if (is_ADC_calibrated)
    {
        if (stat_limit_B > 0)
            return -1;
        if (stat_limit_F > 0)
            return 1;
    }
    return 0;
}

void RotSensor::on_falling_edge()
{
    ++n_rounds;
    if (on_edge != NULL)
        on_edge();
}

void RotSensor::on_rising_edge()
{
    --n_rounds;
    if (on_edge != NULL)
        on_edge();
}

float RotSensor::ADC2res_scale(int n)
{
    float A_max, r, x;
    switch (pot_type)
    {
    case TWO_TERMINALS:
        A_max = (float)ADC_max / n;
        r = X_max * R_0 / (A_max - X_max);
        x = (r - R_min) / (R_max - R_min);
    case THREE_TERMINALS:
        x = (float)(n - ADC_min) / ADC_max;
    }
    if (inverse_ADC)
        return 1. - x;
    else
        return x;
}

int RotSensor::ADC2deg(int n)
{
    float x = ADC2res_scale(n);
    int d = int((x - X_zero) * 360.);
    if(allow_multi_rounds)
        d += n_rounds * 360;
    //return n_rounds * 360 + d;
    return d;
}

int RotSensor::get_deg_now()
{
    return ADC2deg(get_ADC());
}