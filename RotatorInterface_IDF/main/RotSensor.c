#include "RotSensor.h"
#include "Config.h"
#include "motor.h"
// TODO:
// dADC / dt time-based limit switch
// REVERSE
void RotSensor_init(RotSensor* p, PotType pot_type, bool allow_multi_rounds, int brake_engage_defer_us)
{
    p->pot_type = pot_type;
    p->brake_engage_defer_us = brake_engage_defer_us;
    p->update_interval = 10000; // 10ms
    p->updates_per_sec = 1000000L / p->update_interval;
    p->allow_multi_rounds = allow_multi_rounds;
    p->ADC_reading = 0;
    p->degree = DEGREE_INVALID;
    p->ADC_max = ADC_MAX;
    p->ADC_min = ADC_MIN;
    p->ADC_per_degree = (p->ADC_max - p->ADC_min) / 360.;
    p->ADC_halfway = (p->ADC_max + p->ADC_min) >> 1;
    p->ADC_Q1 = (p->ADC_max - p->ADC_min) * 1 / 4 + p->ADC_min;
    p->ADC_T1 = (p->ADC_max - p->ADC_min) * 1 / 3 + p->ADC_min;
    p->ADC_Q3 = (p->ADC_max - p->ADC_min) * 3 / 4 + p->ADC_min;
    p->ADC_T2 = (p->ADC_max - p->ADC_min) * 2 / 3 + p->ADC_min;
    p->inverse_ADC = false;
    p->ADC_zero = 0;
    p->idx_data = 0;
    p->n_data = 0;
    for (int i = 0; i < LEN_FILTER_WINDOW; ++i)
    {
        p->ADC_data[i] = -255;
    }
    p->clipper = -255;
    p->EMA_1 = -255;
    p->EMA_2 = -255;
    p->is_stable = true;
    p->is_ADC_calibrated = false;
    p->ADC_prev_fil = -255;
    p->n_rounds = 0;
    p->n_edge_freeze = 0;
    p->stat_edge_F = 0;
    p->stat_edge_R = 0;
    p->stat_edge_T1 = 0;
    p->stat_edge_T2 = 0;
#ifndef HARD_LIMIT_SWITCH
    p->deg_limit_F = 360 + EXT_RANGE;
    p->deg_limit_B = -EXT_RANGE;
#else
    p->deg_limit_F = 355;
    p->deg_limit_B = 5;
#endif
    p->stat_limit_F = 0;
    p->stat_limit_B = 0;
    p->stat_limit_T = 0;
    p->on_edge = NULL;
    LoopBuffer_init(&p->ADC_results, LEN_RESULTS);
    LoopBuffer_init(&p->deg_results, LEN_RESULTS);
    p->n_ADC_speed = 0;
    p->n_deg_speed = 0;
    p->R_0 = R0_DEFAULT;
    p->R_c = Rc_DEFAULT;
    p->R_max = RMAX_DEFAULT;
    p->R_min = RMIN_DEFAULT;
}

void RotSensor_set_resistors(RotSensor* p, uint32_t R_max, uint32_t R_min, uint32_t R_0, uint32_t R_c)
{
    p->R_max = R_max;
    p->R_min = R_min;
    p->R_0 = R_0;
    p->R_c = R_c;   
}

void RotSensor_set_ADC_range(RotSensor* p, int ADC_inf, int ADC_sup, int ADC_zero, int deg_inf, int deg_sup)
{
    p->ADC_max = ADC_sup;
    p->deg_max = deg_sup;
    p->ADC_min = ADC_inf;
    p->deg_min = deg_inf;
    p->ADC_zero = ADC_zero;
    RotSensor_update_ADC(p);
    // DEBUG
    // Serial.printf("ADC_max=%d,ADC_min=%d,ADC_zero=%d,X_zero=%d,X_sup=%d,X_zero=%d,X_inf=%d,deg_inf=%d,deg_zero=%d,deg_sup=%d\n",
    //               sup, inf, zero,
    //               (int)(X_zero * 100),
    //               (int)(ADC2res_scale(ADC_max) * 100),
    //               (int)(ADC2res_scale(ADC_zero) * 100),
    //               (int)(ADC2res_scale(ADC_min) * 100),
    //               ADC2deg(ADC_min),
    //               ADC2deg(ADC_zero),
    //               ADC2deg(ADC_max));
}

// return 0-1.0 to indicate the absolute position on the potentiometer
static float ADC2res_scale(RotSensor* p, int n)
{
    float A_max, r, x = 0.;
    switch (p->pot_type)
    {
    case TWO_TERMINALS:
        A_max = (float)p->ADC_max / n;
        r = p->X_max * p->R_0 / (A_max - p->X_max);
        x = (r - p->R_min) / (p->R_max - p->R_min);
        break;
    case THREE_TERMINALS:
        x = (float)(n - p->ADC_min) / p->ADC_max;
    }
    if (p->inverse_ADC)
        return 1. - x;
    else
        return x;
}


static int ADC2deg(RotSensor* p, int n)
{
    float x = ADC2res_scale(p, n);
    int d = (int)((x - p->X_zero) * (p->deg_range));
    if(p->allow_multi_rounds)
        d += p->n_rounds * 360;
    //return n_rounds * 360 + d;
    return d;
}

void RotSensor_update_result(RotSensor* p, int r)
{
    LoopBuffer_append(&(p->ADC_results), r);
    p->ADC_reading = r;
    if (true) // is stable
    {
        p->degree = ADC2deg(p, r);
        LoopBuffer_append(&(p->deg_results), p->degree);
        p->n_deg_speed = LoopBuffer_get_latest(&(p->deg_results)) - LoopBuffer_get_oldest(&(p->deg_results));
        p->n_ADC_speed = LoopBuffer_get_latest(&(p->ADC_results)) - LoopBuffer_get_oldest(&(p->ADC_results));
    }
    if (p->degree == DEGREE_INVALID)
        return;

    if (p->degree >= p->deg_limit_F) // reached the forward limit
    {
        if(p->stat_limit_F < LIMIT_STATE_STAGE)
            p->stat_limit_F++;
    }
    else if (p->degree <= p->deg_limit_B) // reached the backward limit
    {
        if(p->stat_limit_B < LIMIT_STATE_STAGE)
            p->stat_limit_B++;
    }
    else // within the range
    {
        if (p->stat_limit_F > 0)
            p->stat_limit_F--;
        if (p->stat_limit_B > 0)
            p->stat_limit_B--;
    }
}


void RotSensor_update_ADC(RotSensor *p) // TODO: modify
{
    p->ADC_halfway = ((p->ADC_max - p->ADC_min) >> 2) + p->ADC_min;
    p->ADC_per_degree = (p->ADC_max - p->ADC_min) / 360.;
    p->ADC_Q1 = (p->ADC_max - p->ADC_min) * 1 / 4 + p->ADC_min;
    p->ADC_T1 = (p->ADC_max - p->ADC_min) * 1 / 3 + p->ADC_min;
    p->ADC_Q3 = (p->ADC_max - p->ADC_min) * 3 / 4 + p->ADC_min;
    p->ADC_T2 = (p->ADC_max - p->ADC_min) * 2 / 3 + p->ADC_min;
    // calibration for TWO_TERMINAL potentiometer
    // calculate X_max, R_min, X_zero
    p->ADC_maxmin = (float)p->ADC_max / p->ADC_min;
    p->X_max = (float)p->R_max / (float)(p->R_max + p->R_0);
    p->R_min = (int)(p->X_max * p->R_0 / (p->ADC_maxmin - p->X_max) + 0.5);
    p->X_min = (float)p->R_min / (float)(p->R_min + p->R_0);
    p->X_zero = ADC2res_scale(p, p->ADC_zero);
    // Serial.printf("A=%d, X_max=%d, R_min=%d, X_min=%d\n",
    //                (int)(ADC_maxmin * 100),
    //                (int)(X_max * 100),
    //                (int)(R_min * 100),
    //                (int)(X_min * 100));
    p->deg_range = (p->deg_max > p->deg_min)?(p->deg_max - p->deg_min):(p->deg_min - p->deg_max);
    p->is_ADC_calibrated = true;
    p->is_stable = true;
}

// call each 50ms
void RotSensor_append_ADC_reading(RotSensor* p, int data)
{
    // clipper
    if (p->clipper < 0) // clipper is not initialized
    {
        p->clipper = data;
    }
    else
    {
        if (data > p->clipper + CLIPPER_DEADZONE)
            p->clipper = p->clipper + CLIPPER_DEADZONE;
        else if (data < p->clipper - CLIPPER_DEADZONE)
            p->clipper = p->clipper - CLIPPER_DEADZONE;
        else
            p->clipper = data;
    } // clipping
    //EMA_1
    if (p->EMA_1 < 0)
        p->EMA_1 = p->clipper;
    else
        p->EMA_1 += (p->clipper - p->EMA_1) / EMA_ALPHA_1;
    //EMA_2
    if (p->EMA_2 < 0)
        p->EMA_2 = p->EMA_1;
    else
        p->EMA_2 += (p->EMA_1 - p->EMA_2) / EMA_ALPHA_2;
    // filtered output EMA_2
    // come here each update_interval * DATA_OVERHEAD = 100ms
    if (p->n_data++ >= DATA_OVERHEAD)
    {
        p->n_data = 0;
        int d = (int)(p->EMA_2);
        RotSensor_update_result(p, d);
        if (p->n_edge_freeze > 0) // falling edge freeze
            p->n_edge_freeze--;
        else if (p->n_edge_freeze < 0) // rising edge freeze
            p->n_edge_freeze++;
        // interval deviation edge detection
        if (LoopBuffer_is_full(&(p->ADC_results)))
        {
            int n_change = (LoopBuffer_get_latest(&(p->ADC_results)) - LoopBuffer_get_oldest(&(p->ADC_results)));
            /* Process the edge problem when pot allow multiple rounds*/
            /* For single round pots, e.g. with hard limit switches, do not care the following codes */
            if(p->allow_multi_rounds)
            {
                // falling edge state machine
                if (p->n_edge_freeze <= 0)
                {
                    if ((p->stat_edge_F < LIMIT_STATE_STAGE))
                    {
                        if (n_change < -EDGE_THRESHOLD)
                        {
                            p->stat_edge_F += 1;
                            p->is_stable = false;
                        }
                        else
                        {
                            p->is_stable = true;
                        }
                    }
                    else if (p->stat_edge_F < 2 * LIMIT_STATE_STAGE)
                    {
                        if (d > p->ADC_Q3)
                        {
                            p->stat_edge_F = 0;
                            p->is_stable = true;
                        }
                        else if (d < p->ADC_Q1)
                        {
                            ++p->stat_edge_F;
                        }
                    }
                    else if (p->stat_edge_F == 2 * LIMIT_STATE_STAGE)
                    {
                        RotSensor_on_falling_edge(p);
                        p->is_stable = true;
                        p->stat_edge_F = 0;
                    }
                }
                // rising edge state machine
                if (p->n_edge_freeze >= 0)
                {
                    if (p->stat_edge_R < LIMIT_STATE_STAGE)
                    {
                        if (n_change > EDGE_THRESHOLD)
                        {
                            ++p->stat_edge_R;
                            p->is_stable = false;
                        }
                        else
                        {
                            p->is_stable = true;
                        }
                    }
                    else if (p->stat_edge_R < 2 * LIMIT_STATE_STAGE)
                    {
                        if (d < p->ADC_Q1)
                        {
                            p->stat_edge_R = 0;
                            p->is_stable = false;
                        }
                        else if (d > p->ADC_Q3)
                        {
                            ++p->stat_edge_R;
                        }
                    }
                    else if (p->stat_edge_R == 2 * LIMIT_STATE_STAGE)
                    {
                        RotSensor_on_rising_edge(p);
                        p->is_stable = true;
                        p->stat_edge_R = 0;
                        p->n_edge_freeze = -EDGE_FREEZE_TIME * p->updates_per_sec / DATA_OVERHEAD;
                    }
                }
            }
        }
    }
}

int RotSensor_get_degree(RotSensor* p)
{
    return LoopBuffer_get_latest(&(p->deg_results));
}

int RotSensor_get_ADC(RotSensor* p)
{
    return LoopBuffer_get_latest(&(p->ADC_results));
}

int RotSensor_is_limit(RotSensor* p)
{
    if (p->is_ADC_calibrated)
    {
        if (p->stat_limit_B > 0)
            return -1;
        if (p->stat_limit_F > 0)
            return 1;
    }
    return 0;
}

void RotSensor_on_falling_edge(RotSensor *p)
{
    ++(p->n_rounds);
    if (p->on_edge != NULL)
        p->on_edge();
}

void RotSensor_on_rising_edge(RotSensor *p)
{
    --(p->n_rounds);
    if (p->on_edge != NULL)
        p->on_edge();
}



int RotSensor_get_deg_now(RotSensor* p)
{
    return ADC2deg(p, RotSensor_get_ADC(p));
}

