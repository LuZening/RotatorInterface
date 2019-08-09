#line 1 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
#line 1 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
#define __DEBUG_
#define __595
// TIMER FREQUENCY = CPU FREQ / 16
#define CYC_PER_US 5
#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>

#include "AT24C.h"
#include "Config.h"
#include "RotSensor.h"
#include "TaskSlot.h"
#include "Lib485.h"
#include "Lib595.h"
#include "motor.h"
#include "commands.h"

#define SGN(X) (((X) < 0) ? (-1) : (((X) == 0) ? (0) : (1)))
#define ABS(X) (((X) >= 0) ? ((X)) : (-(X)))
#define TURN_OFF_LED(p) (SET_PIN(p, HIGH))
#define TURN_ON_LED(p) (SET_PIN(p, LOW))

// TODO: Async webSocket
// pin number of 485 Read/write switch
#define PIN_485RW 5
#define BAUD_RATE 115200

// Limit Switch Input
#define PIN_LIMIT 16

// 74595
#define PIN_DATA 4
#define PIN_SCLK 12
#define PIN_RCLK 13
#define PIN_OE 2

// AT24C64
#define PIN_SDA 14
#define PIN_SCL 0
#define PIN_WP PIN_595(6)

// LEDs
#define PIN_LED_STAT PIN_595(1)
#define PIN_LED_TASK PIN_595(0)

// Relays
#define PIN_PGSW0 PIN_595(5)
#define PIN_PGSW1 PIN_595(4)
#define PIN_PGSW2 PIN_595(3)
#define PIN_PGSW3 PIN_595(2)

// Motor
#define PIN_BREAK PIN_PGSW0
#define PIN_MOTOR_P PIN_PGSW1
#define PIN_MOTOR_N PIN_PGSW2

// sensor
#define POT_TYPE TWO_TERMINALS

// Sched Intervals
#define TIMER_INTERVAL 50000 // us
#define SCHED_INTERVAL TIMER_INTERVAL //us
#define AUTOSAVE_INTERVAL_MS 10000
#define TIMER_PER_SEC 1000000L / SCHED_INTERVAL
// Network
#define MDNS_NAME vortex
#define HTTP_PORT 80
#define WS_PORT 81
IPAddress AP_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// global variables
byte stat_LED_stat = 0; // 0: OFF, 255: ON, 1-254 BLINKS PER SEC
bool level_led_stat = HIGH;
byte stat_LED_task = 0; // 0: OFF, 255: ON, 1-254 BLINKS PER SEC
bool level_led_task = HIGH;
extern struct Task task_slot;
extern struct IO595 *p595;
extern struct EEPROM_AT24C *pEEPROM;
extern struct Serial485 *p485;
extern struct RotSensor *prot_sensor;
struct Task task_slot;
// indicate if it is calirating the motor
bool is_calibrating;
bool is_auto_calibrating;
char n_auto_calibrate_rounds;
float ADC_smoothed;
// count the number of edges when calibrating
unsigned char cal_edge_count;
bool is_config;
// indicate if the device has connected to the router
bool is_wifi_conn;
// indicate if the motor is running
int mot_dir; // -1 CCW +1 CW 0 IDLE
int mot_pwm; // 0-1023
unsigned int time_unsaved_ms;
String s_ip;
char s_httpbuffer[1024];
char s_error[256];
// create rotrary sensor object
RotSensor rot_sensor(POT_TYPE, TIMER_INTERVAL);
RotSensor *prot_sensor = &rot_sensor;
// create motor object
struct Motor motor;
struct Motor *pmotor = &motor;
// create web server objects
//ESP8266WebServer server(HTTP_PORT);
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket webSocket("/ws");
//WebSocketsServer webSocket = WebSocketsServer(WS_PORT);

/********************************************
 *         handle  EEPROM                   *
*/
#line 120 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void sync_active_params(union ActiveWriteBlock *p);
#line 127 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void sync_config(union ConfigWriteBlock *p);
#line 147 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
String getContentType(String filename);
#line 176 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
bool handleFileRead(AsyncWebServerRequest *r);
#line 195 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onSetConfig(AsyncWebServerRequest *r);
#line 311 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onGetConfig(AsyncWebServerRequest *r);
#line 336 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
char * sensorData();
#line 347 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onGetSensor(AsyncWebServerRequest *r);
#line 353 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onTask(AsyncWebServerRequest *r);
#line 439 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onNotFound(AsyncWebServerRequest *r);
#line 460 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onReset(AsyncWebServerRequest *r);
#line 473 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onCalibrateGap(void);
#line 491 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onCalibrate(AsyncWebServerRequest *r);
#line 570 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void begin_auto_calibrate();
#line 582 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void auto_calibration_turn_around();
#line 608 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void log_debug(const char *s);
#line 618 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onTimer();
#line 857 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void init_OTA();
#line 905 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onWebsocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
#line 935 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void setup();
#line 1100 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void loop();
#line 120 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void sync_active_params(union ActiveWriteBlock *p)
{
    p->body.ADC_reading = prot_sensor->ADC_reading;
    p->body.n_degree = prot_sensor->degree;
    p->body.n_rounds = prot_sensor->n_rounds;
}

void sync_config(union ConfigWriteBlock *p)
{
    p->body.pot_type = (int)prot_sensor->pot_type;
    p->body.allow_multi_rounds = prot_sensor->allow_multi_rounds;
    p->body.ADC_min = prot_sensor->ADC_min;
    p->body.ADC_max = prot_sensor->ADC_max;
    p->body.ADC_zero = prot_sensor->ADC_zero;
    p->body.ADC_sensor_gap = prot_sensor->ADC_sensor_gap;
    p->body.deg_limit_CCW = prot_sensor->deg_limit_B;
    p->body.deg_limit_CW = prot_sensor->deg_limit_F;
    p->body.is_ADC_calibrated = prot_sensor->is_ADC_calibrated;
    p->body.R_0 = prot_sensor->R_0;
    p->body.R_c = prot_sensor->R_c;
    p->body.R_max = prot_sensor->R_max;
    p->body.R_min = prot_sensor->R_min;
}

// ******************************************
// ****    handle http requests  ************
// ******************************************
String getContentType(String filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest *r)
{
    String path = r->url();
    if (path.endsWith("/"))
    {
        path += "index.html";
    }

    String contentType = getContentType(path);

    if (SPIFFS.exists(path))
    {
        r->send(SPIFFS, path);
        return true;
    }
    return false;
}

// set config parameters: name, ssid, passwd,
void onSetConfig(AsyncWebServerRequest *r)
{
    int n_params = 0;
    if(r->method == HTTP_POST)
    {
        if (r->hasArg("name"))
        {
            strcpy(p_cfg->body.s_name, r->arg("id").c_str());
            ++n_params;
            is_config = true;
        }
        if (r->hasArg("ssid"))
        {
            strcpy(p_cfg->body.s_ssid, r->arg("ssid").c_str());
            Serial.printf("ssid=%s\r\n", p_cfg->body.s_ssid);
            ++n_params;
            is_config = true;
        }
        if (r->hasArg("passwd"))
        {
            strcpy(p_cfg->body.s_password, r->arg("passwd").c_str());
            ++n_params;
            strcpy(p_cfg->body.s_id, _ID);
            p_cfg->body.is_WiFi_set = true;
            is_config = true;
        }
        if (r->hasArg("pot_type"))
        {
            p_cfg->body.pot_type = r->arg("pot_type").toInt();
            prot_sensor->pot_type = (PotType)p_cfg->body.pot_type;
            ++n_params;
        }
        if(r->hasArg("multi_rounds"))
        {
            p_cfg->body.allow_multi_rounds = r->arg("multi_rounds").toInt();
            prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
            ++n_params;
        }
        if(r->hasArg("R_0"))
        {
            prot_sensor->R_0 = r->arg("R_0").toInt();
            ++n_params;   
        }
        if(r->hasArg("R_c"))
        {
            prot_sensor->R_c = r->arg("R_c").toInt();
            ++n_params;   
        }
        if(r->hasArg("R_max"))
        {
            prot_sensor->R_max = r->arg("R_max").toInt();
            ++n_params;   
        }
        if(r->hasArg("R_min"))
        {
            prot_sensor->R_min = r->arg("R_min").toInt();
            ++n_params;   
        }
        // Calibration Step I
        if (r->hasArg("ADC_max") && r->hasArg("ADC_min") && r->hasArg("ADC_zero")) // Step I
        {
            int ADC_max = r->arg("ADC_max").toInt();
            int ADC_min = r->arg("ADC_min").toInt();
            int ADC_zero = r->arg("ADC_zero").toInt();
            p_cfg->body.ADC_min = ADC_min;
            p_cfg->body.ADC_max = ADC_max;
            p_cfg->body.ADC_zero = ADC_zero;
            prot_sensor->set_ADC_range(ADC_min, ADC_max, ADC_zero);
        }
        // Calibration Step 2
        if (r->hasArg("CW_lim") && r->hasArg("CCW_lim")) // Step 2
        {
            int CW_lim = r->arg("CW_lim").toInt();
            int CCW_lim = r->arg("CCW_lim").toInt();
            p_cfg->body.deg_limit_CW = CW_lim;
            p_cfg->body.deg_limit_CCW = CCW_lim;
            prot_sensor->deg_limit_F = CW_lim;
            prot_sensor->deg_limit_B = CCW_lim;
        }
    }
    else if(r->method == HTTP_GET)
    {
        /* code */
        if(r->hasArg("manual")) // manual calibration
        {
            is_calibrating = true;
        }
        else if(r->hasArg("auto"))
        {
            if ((!is_calibrating) && pmotor->status == MOT_IDLE)
            {
                begin_auto_calibrate();
                r->send(200, "Auto calibration started\n");
            }
            else
            {
                r->send(300, "system is busy");
            }
            return;
        }
        else if(r->hasArg("stop"))
        {
            is_calibrating = false;
            is_auto_calibrating = false;
            cal_edge_count = 0;
            task_slot.type = NULL_TASK;
        }
        else if(r->hasArg("CLR_rounds")) // clear n_rounds = 0
        {
            prot_sensor->n_rounds = 0;
        }
    }
    r->send(SPIFFS, "/config.html");
}

// respond with configurations
void onGetConfig(AsyncWebServerRequest *r)
{
    if (!is_config)
    {
        r->send(200, "text/plain", "badconfig");
        return;
    }
    sprintf(s_httpbuffer, "name=%s&ssid=%s&ip=%s&pot_type=%d&multi_rounds=%d&R_0=%d&R_c=%d&R_max=%d&R_min=%d&ADC_min=%d&ADC_max=%d&ADC_zero=%d&degree=%d&CW_lim=%d&CCW_lim=%d\n",
            p_cfg->body.s_name,
            p_cfg->body.s_ssid,
            s_ip.c_str(),
            (int)prot_sensor->pot_type,
            (int)prot_sensor->allow_multi_rounds,
            prot_sensor->R_0,
            prot_sensor->R_c,
            prot_sensor->R_max,
            prot_sensor->R_min,
            prot_sensor->ADC_min,
            prot_sensor->ADC_max,
            prot_sensor->ADC_zero,
            (int)(prot_sensor->degree),
            prot_sensor->deg_limit_F,
            prot_sensor->deg_limit_B);
    r->send(200, "text/plain", s_httpbuffer);
}
char* sensorData()
{
    sprintf(s_httpbuffer, "azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n",
            prot_sensor->get_degree(),
            prot_sensor->get_ADC(),
            ((pmotor->status == MOT_IDLE) ? (0) : (1)),
            prot_sensor->is_limit(),
            prot_sensor->n_deg_speed);
    return s_httpbuffer;
}
// respond with sensor data
void onGetSensor(AsyncWebServerRequest *r)
{
    r->send(200, "text/plain", sensorData());
}

// recieve task forms from POST request
void onTask(AsyncWebServerRequest *r)
{
    // type: (M)anual OR (T)arget
    // to: CW(+sec) CCW(-sec) OR degree (0-360)
    // speed: (0-255)
    if (r->hasArg("type") && r->hasArg("to") && r->hasArg("speed"))
    {
        float deg = prot_sensor->degree;
        int type = r->arg("type").toInt();
        int to = r->arg("to").toInt();
        int to_2, to_3;
        int speed = r->arg("speed").toInt();
        // create a task object
        if ((speed >= 0 && speed <= 255) && (type == MANUAL || type == TARGET || type == NULL_TASK))
        {
            if (type == MANUAL &&
                to <= MANUAL_INTERVAL &&
                to >= -MANUAL_INTERVAL) // Manual control
            {
                // create a manual control task object
                task_slot.type = MANUAL;
                task_slot.n_to = to;
                task_slot.n_speed = speed;
                task_slot.is_executed = false;
                log_debug("Manual task recieved\n");
            }
            else if (type == TARGET && to >= 0 && to <= 360) // To a target
            {
                task_slot.type = TARGET;
                task_slot.n_speed = speed;
                // put TO into the range of limit switches
                if (to > prot_sensor->deg_limit_F)
                {
                    while (to > prot_sensor->deg_limit_F)
                        to -= 360;
                    if (to >= prot_sensor->deg_limit_B) // found a solution
                    {
                        to_2 = ((to - 360 >= prot_sensor->deg_limit_B) ? (to - 360) : (to));
                    }
                    else // cannot find a solution
                    {
                        to = prot_sensor->deg_limit_F;
                        to_2 = prot_sensor->deg_limit_B;
                    }
                }
                else if (to < prot_sensor->deg_limit_B)
                {
                    while (to < prot_sensor->deg_limit_B)
                        to += 360;
                    if (to <= prot_sensor->deg_limit_F)
                    {
                        to_2 = ((to + 360 <= prot_sensor->deg_limit_F) ? (to + 360) : (to));
                    }
                    else
                    {
                        to = prot_sensor->deg_limit_B;
                        to_2 = prot_sensor->deg_limit_F;
                    }
                }
                else
                {
                    to_2 = ((to - 360 > prot_sensor->deg_limit_B) ? (to - 360) : (to));
                    to_3 = ((to + 360 < prot_sensor->deg_limit_F) ? (to + 360) : (to));
                    to_2 = ((ABS(to_2 - deg) < ABS(to_3 - deg)) ? (to_2) : (to_3));
                }
                task_slot.n_to = ((ABS(to_2 - deg) < ABS(to - deg)) ? (to_2) : (to));
                task_slot.is_executed = false;
                log_debug("Target task recieved\n");
            }
            else if (type == NULL_TASK)
            {
                task_slot.type = NULL_TASK;
                task_slot.is_executed = false;
                task_slot.n_to = 0;
            }
            r->send(200, "text/plain", "task created\n");
        }
        else
        {
            r->send(400, "text/plain", "invalid parameters\n");
        }
    }
    r->send(400, "text/plain", "Bad request\n");
}

// deprecated, using server.serveStatic
void onNotFound(AsyncWebServerRequest *r)
{
    if (!handleFileRead(r)) // if no file matches the request, return with ERROR 404
    {
        // TODO: static string
        String message = "AsyncServer: File Not Found\n\n";
        message += "URI: ";
        message += r->url();
        message += "\nMethod: ";
        message += (r->method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += r->args();
        message += "\n";
        for (uint8_t i = 0; i < r->args(); i++)
        {
            message += " " + r->argName(i) + ": " + r->arg(i) + "\n";
        }
        r->send(200, "text/plain", message);
    }
}

void onReset(AsyncWebServerRequest *r)
{
    r->send(200, "text/plain", "Restarting...\n");
    Serial.println("Reset...");
    sync_config(p_cfg);
    sync_active_params(p_actprm);
    save_config(p_cfg);
    save_active_params(p_actprm);
    SPIFFS.end();
    delay(1000);
    ESP.reset();
}

void onCalibrateGap(void) // the callback function when a gap has been detected
{
    p_cfg->body.ADC_sensor_gap = prot_sensor->ADC_reading;
    ++cal_edge_count;
    if (cal_edge_count == 2)
    {
        prot_sensor->set_ADC_range(p_cfg->body.ADC_min, p_cfg->body.ADC_max, p_cfg->body.ADC_zero);
    }
    else if (cal_edge_count == 3)
    {
        cal_edge_count = 0;
        task_slot.type = NULL_TASK; // stop the motor
        os_printf("Calibration finished\n");
        prot_sensor->on_edge = NULL;
    }
}

//TODO: start calibration, end calibration ...
void onCalibrate(AsyncWebServerRequest *r)
{
    if (r->method() == HTTP_POST)
    {
        String message = "ADC calibrate:";
        if (r->hasArg("ADC_max") && r->hasArg("ADC_min") && r->hasArg("ADC_zero")) // Step I
        {
            int ADC_max = r->arg("ADC_max").toInt();
            int ADC_min = r->arg("ADC_min").toInt();
            int ADC_zero = r->arg("ADC_zero").toInt();
            message += "\nADC_max: ";
            message += ADC_max;
            message += "\nADC_min: ";
            message += ADC_min;
            message += "\nADC_zero: ";
            message += ADC_zero;
            // if (ADC_max <= 1 || ADC_max > ADC_RANGE)
            //     message += "ADC max invalid";
            // if (ADC_min < 0 || ADC_min > ADC_RANGE)
            //     message += "ADC min invalid";
            // if (ADC_zero < ADC_min || ADC_zero > ADC_max)
            //     message += "ADC max invalid";
            p_cfg->body.ADC_min = ADC_min;
            p_cfg->body.ADC_max = ADC_max;
            p_cfg->body.ADC_zero = ADC_zero;
            prot_sensor->set_ADC_range(ADC_min, ADC_max, ADC_zero);
            message += "\n\n";
            r->send(200, message);
        }
        if (r->hasArg("CW_lim") && r->hasArg("CCW_lim")) // Step 2
        {
            int CW_lim = r->arg("CW_lim").toInt();
            int CCW_lim = r->arg("CCW_lim").toInt();
            message += "\nCW_lim: ";
            message += CW_lim;
            message += "\nCCW_lim: ";
            message += CCW_lim;
            p_cfg->body.deg_limit_CW = CW_lim;
            p_cfg->body.deg_limit_CCW = CCW_lim;
            prot_sensor->deg_limit_F = CW_lim;
            prot_sensor->deg_limit_B = CCW_lim;
            message += "\n\n";
            r->send(200, message);
        }
    }
    else if(r->hasArg("manual"))// manual calibration mode 
    {
        is_calibrating = true;
        r->redirect("/calibrate");
    }
    else if (r->hasArg("auto")) // GET request to initialize calibration
    {
            if ((!is_calibrating) && pmotor->status == MOT_IDLE)
            {
                begin_auto_calibrate();
                r->send(200, "Auto calibration started\n");
            }
            else
            {
                r->send(300, "system is busy");
            }
    }
    else if(r->hasArg("stop"))
    {
        is_calibrating = false;
        sync_active_params(p_actprm);
        sync_config(p_cfg);
        save_active_params(p_actprm);
        save_config(p_cfg);
        r->send(200, "saved\n");
    }
    else
    {
        is_calibrating = true;
        r->send(SPIFFS, "/calibrate.html");
    }
    
}

void begin_auto_calibrate()
{
    // set current point as North point
    prot_sensor->ADC_zero = prot_sensor->get_ADC();
    is_calibrating = true;
    is_auto_calibrating = true;
    // post a CW manual task
    n_auto_calibrate_rounds = 0;
    set_task(&task_slot, MANUAL, INFINITE_TIME, MAX_SPEED);
}

// motor turns around when it runs to the limit
void auto_calibration_turn_around()
{
    // 0CW,1CCW,2CW,3CCW->ORIGIN
    ++n_auto_calibrate_rounds;
    switch (n_auto_calibrate_rounds)
    {
    case 1: // CCW
        set_task(&task_slot, MANUAL, -INFINITE_TIME, MAX_SPEED);
        break;
    case 2: //CW
        set_task(&task_slot, MANUAL, INFINITE_TIME, MAX_SPEED);
        break;
    case 3: //CCW
        prot_sensor->update_ADC();
        prot_sensor->deg_limit_F = prot_sensor->get_deg_now();
        set_task(&task_slot, MANUAL, -INFINITE_TIME, MAX_SPEED);
        break;
    case 4: // FINISHED, back to origin
        prot_sensor->deg_limit_B = prot_sensor->get_deg_now();
        set_task(&task_slot, TARGET, 0, MAX_SPEED);
        is_auto_calibrating = false;
        is_calibrating = false;
    }
}


void log_debug(const char *s)
{
#ifdef __DEBUG_
    Serial.print(s);
#endif
}

/*********************************************
 *          Main Scheduler                   *
 * ******************************************/
void onTimer() // 50ms
{
    static unsigned int tick = 0;
    static unsigned int tick_time_limit = 0;
    static int degree_lastsec = -1000;
    static unsigned char flash_LED = LOW;
    unsigned long time_s = micros();
    ESP.wdtFeed();
    //increment tick
    ++tick;
    // check WiFi status
    // 2ms/% 1%0(TIMER_PER_SEC / ) 
    // BLINK LED
    switch (stat_LED_stat)
    {
    case 0:
        TURN_OFF_LED(PIN_LED_STAT);
        break;
    case 255:
        TURN_ON_LED(PIN_LED_STAT);
        break;
    default:
        if (tick % (TIMER_PER_SEC / stat_LED_stat) == 0)
        {
            level_led_stat = !level_led_stat;
            SET_PIN(PIN_LED_STAT, level_led_stat);
        }
    }

    switch (stat_LED_task)
    {
    case 0:
        TURN_OFF_LED(PIN_LED_TASK);
        break;
    case 255:
        TURN_ON_LED(PIN_LED_TASK);
        break;
    default:
        if (tick % (TIMER_PER_SEC / stat_LED_task) == 0)
        {
            level_led_task = !level_led_task;
            SET_PIN(PIN_LED_TASK, level_led_task);
        }
    }

    // set TASK LED
    if (task_slot.type == NULL_TASK)
        stat_LED_task = 0;
    else
        stat_LED_task = 255;

    // Autosave EEPROM
    time_unsaved_ms += SCHED_INTERVAL / 1000;
    if (time_unsaved_ms >= AUTOSAVE_INTERVAL_MS)
    {
        time_unsaved_ms = 0;
        int n_deg_diff = prot_sensor->degree - p_actprm->body.n_degree;
        if (abs(n_deg_diff) >= MOT_DEADZONE)
        {
            sync_active_params(p_actprm);
            save_active_params(p_actprm);
        }
    }

    // Broadcast sensor Data 2/sec
    // if(tick % 300 == 0 && webSocket.connectedClients())
    // {
    //     webSocket.broadcastTXT(sensorData());
    // }
    if(tick % (TIMER_PER_SEC / 2) == 1 && webSocket.count())
    {
        webSocket.textAll(sensorData());
    }
    // TASK 1 update ADC
    prot_sensor->append_ADC_reading(ADC_smoothed);
    // TASK 1.5 Calibrating ADC
    if (is_auto_calibrating)
    {
        int n = prot_sensor->ADC_reading;
        if (n > prot_sensor->ADC_max)
            prot_sensor->ADC_max = n;
        else if (n < prot_sensor->ADC_min)
            prot_sensor->ADC_min = n;
    }
    // TASK 2 toggle limit protection
    if ((!is_calibrating) && (pmotor->status == prot_sensor->is_limit()) && (prot_sensor->is_limit() != 0))
    {
        // stop the task
        log_debug("LOG: Limit triggered, attempt to stop\n");
        if (stop_motor(pmotor))
        {
            task_slot.type = NULL_TASK;
        }
    }
    // timeout limit protection each 1 sec
    if ((tick % TIMER_PER_SEC == 3) &&
        (pmotor->status != MOT_IDLE) &&
        task_slot.type != NULL_TASK)
    {
        Serial.printf("Deg speed: %d\n", prot_sensor->n_deg_speed);
        if (abs(prot_sensor->n_deg_speed) <= 3 || SGN(prot_sensor->n_deg_speed) != pmotor->status)
        {

            // timeout limit protection triggered
            if (++(prot_sensor->stat_limit_T) >= 3)
            {
                prot_sensor->stat_limit_T = 0;
                log_debug("LOG: Time limit triggered, attempt to stop\n");
                while (!stop_motor(pmotor));
                task_slot.type = NULL_TASK;
                if (is_calibrating && is_auto_calibrating)
                {
                    auto_calibration_turn_around();
                }
            }
        }
        else
        {
            prot_sensor->stat_limit_T = 0;
        }
    }
    // TASK 3 judge if the task is completed
    int deg_now = prot_sensor->degree;
    if (task_slot.type == MANUAL)
    {
        if (task_slot.n_to > 0) // time decay of manual tasks
        {
            int n_to_next = task_slot.n_to - SCHED_INTERVAL / 1000;
            task_slot.n_to = ((n_to_next < 0) ? (0) : (n_to_next));
        }
        else if (task_slot.n_to < 0)
        {
            int n_to_next = task_slot.n_to + SCHED_INTERVAL / 1000;
            task_slot.n_to = ((n_to_next > 0) ? (0) : (n_to_next));
        }
        else // when the life time of the manual task runs out, kill the task
        {
            task_slot.type = NULL_TASK;
            log_debug("Task completed\n");
            while (!stop_motor(pmotor))
                ;
            return;
        }
    }
    else if (task_slot.type == TARGET)
    {
        if ((pmotor->status == MOT_CW && deg_now >= task_slot.n_to) || (pmotor->status == MOT_CCW && deg_now <= task_slot.n_to))
        {
            if (stop_motor(pmotor)) // return true if the motor stopping signial is confirmed
            {
                task_slot.type = NULL_TASK;
                log_debug("Task completed\n");
                return;
            }
        }
    }
    accelerate(pmotor);
    // TASK 4 execute the task
    if (!task_slot.is_executed)
    {
        switch (pmotor->status)
        {
        case MOT_IDLE: // initiate a new task
            switch (task_slot.type)
            {
            case MANUAL:
                if (is_calibrating ||
                    (prot_sensor->is_ADC_calibrated &&
                     SGN(task_slot.n_to) != prot_sensor->is_limit()))
                {
                    if (task_slot.n_to > 0) // CW
                    {
                        run_CW(pmotor);
                    }
                    else if (task_slot.n_to < 0) // CCW
                    {
                        run_CCW(pmotor);
                    }
                    task_slot.is_executed = true;
                    log_debug("Manual task started\n");
                }
                else // kill the task if limit switches are toggled
                {
                    if (stop_motor(pmotor))
                    {
                        log_debug("Task killed for limits\n");
                        task_slot.type = NULL_TASK;
                    }
                }
                break;
            case TARGET:
                if (prot_sensor->is_ADC_calibrated)
                {
                    // CCW
                    if (task_slot.n_to < deg_now - MOT_DEADZONE)
                    {
                        run_CCW(pmotor);
                        log_debug("Target task created\n");
                    }
                    // CW
                    else if (task_slot.n_to > deg_now + MOT_DEADZONE)
                    {
                        run_CW(pmotor);
                        log_debug("Target task created\n");
                    }
                    // DEADZONE
                    else
                    {
                        task_slot.type = NULL_TASK;
                        log_debug("Task killed for deadzone\n");
                    }
                    task_slot.is_executed = true;
                }
                else
                {
                    task_slot.type = NULL_TASK;
                    task_slot.is_executed = true;
                }
                break;
            case NULL_TASK:
                task_slot.is_executed = true;
            }

            break;
        default: // alter the task
            if ((task_slot.type == MANUAL && SGN(task_slot.n_to) != pmotor->status) ||
                (task_slot.type == TARGET && SGN(task_slot.n_to - deg_now) != pmotor->status) ||
                task_slot.type == NULL_TASK)
            {
                while (!stop_motor(pmotor))
                    ; // force stopping
                task_slot.type = NULL_TASK;
                log_debug("LOG: task cancelled\n");
            }
        }
    }
}

// OTA init
void init_OTA()
{
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}

void onWebsocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        os_printf("WS connected %d\n", server->count());
        client->ping();
    }
    else if(type == WS_EVT_DISCONNECT)
    {
        os_printf("WS disconnected %d\n", client->id());
    }
    else if(type == WS_EVT_PONG)
    {
        os_printf("WS PONG %d\n", client->id());
    }
    else if(type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if(info->opcode == WS_TEXT)
        {
            os_printf("WS recved: %s\n", (char*)data);
        }
        else
        {

            os_printf("WS recved\n");
        }
    }
}

void setup()
{
    //************* setting Pins         ****************//
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_SCLK, OUTPUT);
    pinMode(PIN_RCLK, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    SET_PIN(PIN_OE, HIGH);
    pinMode(PIN_SDA, OUTPUT_OPEN_DRAIN);
    pinMode(PIN_SCL, OUTPUT);
    pinMode(PIN_485RW, OUTPUT);
    pinMode(PIN_LIMIT, INPUT_PULLUP);

    //************** initializing vars **************//
    stat_LED_stat = 0;
    stat_LED_task = 0;
    is_calibrating = false;
    is_auto_calibrating = false;
    n_auto_calibrate_rounds = 0;
    cal_edge_count = 0;
    is_config = false;
    is_wifi_conn = false;
    mot_dir = 0;
    mot_pwm = 0;
    time_unsaved_ms = 0;
    //************** initializing objects **************//
    // 485
    begin_serial485(p485, &Serial, BAUD_RATE, PIN_485RW);
    //595
    begin_595(p595, PIN_SCLK, PIN_RCLK, PIN_OE, PIN_DATA);
    // EEPROM
    begin_AT24C(pEEPROM, AT24C64, PIN_SCL, PIN_SDA, PIN_WP, 0);
    // task slot
    init_task(&task_slot);
    // motor

    init_motor(pmotor, PIN_BREAK, PIN_MOTOR_N, PIN_MOTOR_P);
    // SPIFFS
    SPIFFS.begin();
    //************* read config from EEPROM ****************//
    // initialize config/param datastructures
    load_config(p_cfg);
    load_active_params(p_actprm);
    Serial.printf("ID: %s\n", p_cfg->body.s_id);
    if (strcmp(p_cfg->body.s_id, _ID) == 0) // check if config is valid
    {
        Serial.println("Configuration found!");
        /** configuration found  **/
        // load ADC settings
        // read sensors to get ADC now
        unsigned int ADC_now = 0;
        unsigned int ADC_stored = p_actprm->body.ADC_reading;
        // obtain initial readings
        while (!prot_sensor->ADC_results->is_full())
        {
            prot_sensor->append_ADC_reading(analogRead(A0));
            delay(TIMER_INTERVAL / 1000);
        }
        prot_sensor->is_stable = true;
        while (!prot_sensor->deg_results->is_full())
        {
            prot_sensor->append_ADC_reading(analogRead(A0));
            delay(TIMER_INTERVAL / 1000);
        }
        ADC_smoothed = (float)analogRead(A0);

        // Use loaded config to initialize the sensor
        prot_sensor->pot_type = (enum PotType)p_cfg->body.pot_type;
        prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
        prot_sensor->ADC_max = p_cfg->body.ADC_max;
        prot_sensor->ADC_min = p_cfg->body.ADC_min;
        prot_sensor->ADC_zero = p_cfg->body.ADC_zero;
        prot_sensor->is_ADC_calibrated = p_cfg->body.is_ADC_calibrated;
        prot_sensor->R_0 = p_cfg->body.R_0;
        prot_sensor->R_c = p_cfg->body.R_c;
        prot_sensor->R_min = p_cfg->body.R_min;
        prot_sensor->R_max = p_cfg->body.R_max;
        prot_sensor->ADC_reading = p_actprm->body.ADC_reading;
        prot_sensor->degree = p_actprm->body.n_degree;
        prot_sensor->n_rounds = p_actprm->body.n_rounds;
        prot_sensor->update_ADC();

        // conect to WiFi
        // Serial.printf("Attempt to connect to designated WiFi router...\n");
        if (1)
        {
            Serial.printf("Connecting STA %s\r\n", p_cfg->body.s_ssid);
            WiFi.mode(WIFI_STA);
            WiFi.begin(p_cfg->body.s_ssid, p_cfg->body.s_password);
            int n_conn_try = 0;
            while ((WiFi.status() != WL_CONNECTED) && (n_conn_try++ < 5))
            {
                delay(1000);
            }
            if (WiFi.status() == WL_CONNECTED)
            {
                s_ip = WiFi.localIP().toString();
                log_debug("WiFi connected  ");
                log_debug(s_ip.c_str());
                is_config = true;
                is_wifi_conn = true;
                p_cfg->body.is_WiFi_set = true;
                // Turn on the LED
                stat_LED_stat = 1;
                TURN_ON_LED(PIN_LED_STAT);
            }
            else // falied connecting to WiFi
            {
                Serial.println("STA connection failed");
                p_cfg->body.is_WiFi_set = false;
            }
        }
    }
    else // not correctly configured. Initialize an Acess Point for configuration
    {
        log_debug("ID mismatch");
        is_config = false;
        prot_sensor->is_ADC_calibrated = false;
        // empty the uninitialized strings
        p_cfg->body.s_id[0] = 0;
        p_cfg->body.s_name[0] = 0;
        p_cfg->body.s_password[0] = 0;
        p_cfg->body.s_ssid[0] = 0;
        p_cfg->body.is_ADC_calibrated = false;
        p_cfg->body.is_WiFi_set = false;
    }
    // if falied to connect WiFi as STA, initiate an AP hotspot
    ESP.wdtEnable(2000);
    if (!p_cfg->body.is_WiFi_set)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(AP_IP, gateway, subnet);
        if (WiFi.softAP("VORTEX1"))
        {
            log_debug("AP started");
            s_ip = WiFi.softAPIP().toString();
            stat_LED_stat = 4;
            while(WiFi.softAPgetStationNum() == 0){log_debug("Waiting for connections\n");ESP.wdtFeed();delay(1000);};
            delay(1000);
        }
    }
    // initiate HTTP service
    server.onNotFound(onNotFound);
    server.on("/calibrate", onCalibrate); 
    server.on("/setconfig", HTTP_POST, onSetConfig);
    server.on("/getconfig", onGetConfig);
    server.on("/reset", onReset);
    server.on("/getSensor", onGetSensor);
    server.on("/task", HTTP_POST, onTask);
    //server.serveStatic("/", SPIFFS, "/");
    server.begin();
    // initialize WS service
    webSocket.onEvent(onWebsocketEvent);
    server.addHandler(&webSocket);
    // initialize OTA
    init_OTA();
    while (!prot_sensor->is_stable)
    {
        prot_sensor->append_ADC_reading(analogRead(A0));
        delay(SCHED_INTERVAL / 1000);
    }
    Serial.print(s_ip);
}
#define ADC_SMOOTHING_PERIOD  16
#define ADC_SMOOTHING_CONST  (2.0 / (ADC_SMOOTHING_PERIOD + 1))
void loop()
{
    static unsigned int ticker = micros();
    static unsigned int ticker_ADC = ticker; // a 2ms ticker for ADC smoothing
    unsigned long ticker_new = micros(); 
    unsigned long ticker_ADC_new = ticker_new;
    // using EMA to smooth ADC readings
    if(ticker_ADC_new - ticker >= 10000)
    {
        int ADC_reading = analogRead(A0);
        ticker_ADC = ticker_ADC_new;
        ADC_smoothed = (ADC_reading - ADC_smoothed) * ADC_SMOOTHING_CONST  + ADC_smoothed;
    }
    if (ticker_new - ticker >= SCHED_INTERVAL)
    {
        ticker = ticker_new;
        onTimer();
    }
    ArduinoOTA.handle();
}
// TODO:
// ADC averaging
