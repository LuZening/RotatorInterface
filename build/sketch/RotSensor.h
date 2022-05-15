#line 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\RotSensor.h"
#ifndef __ROTARY_SENSOR_
#define __ROTARY_SENSOR_

#define HARD_LIMIT_SWITCH

#ifndef SCHED_INTERVAL
#define SCHED_INTERVAL 2000
#endif
#define SAMPLE_RATE  1000000L / TIMER_INTERVAL
#define ADC_MAX 900
#define ADC_MIN 20
#define ADC_RANGE 1024 // 10 bit ADC
#define LEN_FILTER_WINDOW 5
#define LEN_RESULTS 20      // window size N*100ms
#define EXT_RANGE 45         // extended range: 45DEG each side
#define CLIPPER_DEADZONE 100 //DEADZONE SET to +- 100LSB
#define EMA_ALPHA_1 4       // 1/4
#define EMA_ALPHA_2 2       // 1/2
#define DATA_OVERHEAD 2     // overhead data needed before generating stable output
#define EDGE_AVG_RANGE 3     // the range of average edge detector
#define EDGE_DATEBACK 20
#define EDGE_THRESHOLD 100
#define EDGE_STATE_TIME 30  // Time to live of the edge detector (UNIT: DATAOVERHEAD * SCHED_INTERVAL ms)
#define EDGE_STATE_STAGE 5  // number of stages the edge detectors have
#define EDGE_FREEZE_TIME 10 // time freeze edge detector/seconds
#define EDGE_THRESHOLD 100
#define LIMIT_STATE_STAGE 3 // number of stages of limit switches
#define DEGREE_INVALID -32767
// resistor configuration
#define R0_DEFAULT 1000
#define Rc_DEFAULT 0
#define RMAX_DEFAULT 500
#define RMIN_DEFAULT 20
#include "Config.h"
#include <Arduino.h>
#include "LoopBuffer.h"

enum PotType
{
    TWO_TERMINALS=0,
    THREE_TERMINALS
};
class RotSensor
{
  public:
    /* System parameters */
    unsigned long update_interval; // us
    unsigned int updates_per_sec;
    /*  Potentiometer parameters    */
    // type 
    enum PotType pot_type; // SAVE
    bool allow_multi_rounds; // SAVE
    // resistors
    unsigned int R_0; // SAVE: pre-dividing resistor 500Ohm
    unsigned int R_c; // SAVE: zero-level elevation resistor 51Ohm
    unsigned int R_max; // SAVE: potentiometer resistance range
    /*    ADC parameters     */
    // the direct numerical reading from ADC
    int ADC_reading;
    int degree; // invalid degree = -MAX_INT
    // the upperbound of ADC_reading
    int ADC_max; // SAVE
    // the lowerbound of ADC_reading
    int ADC_min; // SAVE
    // (ADC_max - ADC_min)>>2 + ADC_min
    int ADC_halfway;
    // 1/4 range
    int ADC_Q1;
    // 3/4 range
    int ADC_Q3;
    // 1/3 range
    int ADC_T1;
    // 2/3 range
    int ADC_T2;
    float ADC_per_degree;
    // 0 degree calibration point
    int ADC_zero; // SAVE
    bool inverse_ADC; // SAVE if ADC is inversed
    float X_max; // X_max = R_max / (R_max + R0)
    float X_min; // X_min = R_min / (R_min + R0)
    float ADC_maxmin; // ADC_maxmin = ADC_max / ADC_min
    float X_zero; // the 0-1 relative scale on the potentiometer at 0 degree point
    unsigned int R_min; // potentiometer min resistance
    int ADC_sensor_gap;
    // list of low frequency ADC readings 100ms interval
    int ADC_data[LEN_FILTER_WINDOW];
    /* Input filter   */
    // input clipper
    int clipper;
    // EMA 1
    float EMA_1;
    // EMA 2
    float EMA_2;
    int idx_data;
    int n_data;
    int n_edge_freeze;
    // state machine for edge detection
    // stat>0 falling edge
    // stat<0 rising edge
    // edge state machine timer 1
    int stat_edge_T1;
    // edge state machine timer 2
    int stat_edge_T2;
    // rising edge detection
    int stat_edge_R;
    // falling edge detection
    int stat_edge_F;
    // list of filtered results
    LoopBuffer *ADC_results;
    // list of angular position records in degree
    LoopBuffer *deg_results;
    int n_deg_speed;
    int n_ADC_speed;
    // previous filtered ADC_data
    int ADC_prev_fil;
    // number of complete rounds
    int n_rounds; // SAVE
    // get filtered reading after clipper in ADC value (ADC_min - ADC_max)
    // stat_limit_T: time_out limit switch 
    int deg_limit_F, deg_limit_B; // SAVE: forward (CW) limit degree, backward (CCW) limit degree
    int stat_limit_F, stat_limit_B, stat_limit_T;
    // update averaged result
    bool is_ADC_calibrated; // SAVE
    bool is_stable;        // indicate if stable output is ready
    void (*on_edge)(void); // callback function when edge detected
    
  public:
    RotSensor(PotType pot_type, unsigned long update_interval);
    void update_result(int r);
    // set ADC lower/upper bound and zeropoint
    void set_ADC_range(int inf, int sup, int zero);
    // calculate ADC parameters after resetting the parameters
    void update_ADC();
    void initialize_ADC(int n_rounds, int degree);
    // append ADC reading
    void append_ADC_reading(int);
    // return true is limit switch has been triggered -1: backward limit 1: forward limit
    int is_limit();
    void on_falling_edge();
    void on_rising_edge();
    int get_degree();
    int get_ADC();
    int get_deg_now();
  private:
    float ADC2res_scale(int n); // 0-1 relative resistance scale
    int ADC2deg(int n);
    int deg2ADC(int deg);
};



#endif
