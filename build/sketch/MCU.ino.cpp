#include <Arduino.h>
#line 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"

#define ESP8266
#define __595

//#define __DEBUG_
// TIMER FREQUENCY = CPU FREQ / 16
#define CYC_PER_US 5
#include "my_types.h"
#include <pgmspace.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#ifdef __DEBUG_
#include <GDBStub.h>
#endif

#include "AT24C.h"
#include "Config.h"
#include "RotSensor.h"
#include "TaskSlot.h"
#include "Lib485.h"
#include "Lib595.h"
#include "motor.h"

// SPIFFS is deprecated, move to LittleFS since ESP8266 Arduino 3.0.2
#define USE_LittleFS
#ifdef USE_LittleFS
#include <FS.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#endif

#define SGN(X) (((X) < 0) ? (-1) : (((X) == 0) ? (0) : (1)))
#define ABS(X) (((X) >= 0) ? ((X)) : (-(X)))
#define TURN_OFF_LED(p) (SET_PIN(p, HIGH))
#define TURN_ON_LED(p) (SET_PIN(p, LOW))

// pin number of 485 Read/write switch
#define PIN_485RW 5

#ifdef __DEBUG_
#define BAUD_RATE 115200
#else
#define BAUD_RATE 9600
#endif

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
#define TIMER_INTERVAL 50000          // us
#define SCHED_INTERVAL TIMER_INTERVAL //us
#define AUTOSAVE_INTERVAL_MS 60000
#define TIMER_PER_SEC (1000000L / SCHED_INTERVAL)
// Network
#define MDNS_NAME vortex
#define HTTP_PORT 80
#define WS_PORT 81
IPAddress AP_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
#ifdef __DEBUG_
#define LOG_DEBUG(fmt, ...) Serial.printf_P(PSTR(fmt), ##__VA_ARGS__)
#elif __WEB_DEBUG_
#define LOG_DEBUG(fmt, ...)          \
    do                               \
    {                                \
        webSocket.textAll(PSTR(fmt)) \
    } while (0)
#else
#define LOG_DEBUG(...) \
    do                 \
    {                  \
        (void)0;       \
    } while (0)
#endif
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
struct Task task_slot; //global
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
#line 152 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void sync_active_params(union ActiveWriteBlock *p);
#line 159 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void sync_config(union ConfigWriteBlock *p);
#line 180 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
String getContentType(String filename);
#line 209 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
bool handleFileRead(AsyncWebServerRequest *r, String path);
#line 235 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onSetConfig(AsyncWebServerRequest *r);
#line 360 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onGetConfig(AsyncWebServerRequest *r);
#line 391 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
char * sensorData();
#line 403 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onGetWiFiStatus(AsyncWebServerRequest *r);
#line 421 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onGetSensor(AsyncWebServerRequest *r);
#line 427 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onTask(AsyncWebServerRequest *r);
#line 513 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onHomepage(AsyncWebServerRequest *r);
#line 525 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onDebug485(AsyncWebServerRequest *r);
#line 530 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onWebConfig(AsyncWebServerRequest *r);
#line 535 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onStatus(AsyncWebServerRequest *r);
#line 544 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onWebGetRS485(AsyncWebServerRequest *r);
#line 559 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onWebPostRS485(AsyncWebServerRequest *r);
#line 628 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onNotFound(AsyncWebServerRequest *r);
#line 658 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onReset(AsyncWebServerRequest *r);
#line 671 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onCalibrateGap(void);
#line 687 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onCalibrate(AsyncWebServerRequest *r);
#line 765 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void begin_auto_calibrate();
#line 777 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void auto_calibration_turn_around();
#line 805 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onTimer();
#line 1060 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void init_OTA();
#line 1108 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void onWebsocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
#line 1141 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void setup();
#line 1319 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
void loop();
#line 152 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
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
    p->body.inverse_ADC = prot_sensor->inverse_ADC;
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

bool handleFileRead(AsyncWebServerRequest *r, String path)
{
    if (path.endsWith(F("/")))
    {
        path += F("index.html");
    }

    String contentType = getContentType(path);
    String path_gz(path);
    path_gz += (".gz");
    if (SPIFFS.exists(path_gz))
    {
        AsyncWebServerResponse *response = r->beginResponse(SPIFFS, path_gz, contentType);
        response->addHeader(("Content-Encoding"), ("gzip"));
        r->send(response);
        return true;
    }
    else if (SPIFFS.exists(path))
    {
        r->send(SPIFFS, path, contentType);
        return true;
    }
    return false;
}

// set config parameters: name, ssid, passwd,
void onSetConfig(AsyncWebServerRequest *r)
{
    int n_params = 0;
    if (r->method() == HTTP_POST)
    {
        if (r->hasArg(F("name")))
        {
            strcpy(p_cfg->body.s_name, r->arg(F("name")).c_str());
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(F("ssid")))
        {
            strcpy(p_cfg->body.s_ssid, r->arg(F("ssid")).c_str());
            // Serial.printf("ssid=%s\r\n", p_cfg->body.s_ssid);
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(F("passwd")))
        {
            strcpy(p_cfg->body.s_password, r->arg(F("passwd")).c_str());
            ++n_params;
            strcpy(p_cfg->body.s_id, _ID);
            p_cfg->body.is_WiFi_set = true;
            is_config = true;
        }
        if (r->hasArg(F("pot_type")))
        {
            p_cfg->body.pot_type = r->arg(F("pot_type")).toInt();
            prot_sensor->pot_type = (PotType)p_cfg->body.pot_type;
            ++n_params;
        }
        if (r->hasArg(F("multi_rounds")))
        {
            p_cfg->body.allow_multi_rounds = (r->arg(F("multi_rounds")).toInt() > 0);
            prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
            ++n_params;
        }
        if (r->hasArg(F("break_delay")))
        {
            p_cfg->body.break_engage_defer = r->arg(F("break_delay")).toInt();
            ++n_params;
        }
        if (r->hasArg(F("R_0")))
        {
            prot_sensor->R_0 = r->arg(F("R_0")).toInt();
            ++n_params;
        }
        if (r->hasArg(F("R_c")))
        {
            prot_sensor->R_c = r->arg(F("R_c")).toInt();
            ++n_params;
        }
        if (r->hasArg(F("R_max")))
        {
            prot_sensor->R_max = r->arg(F("R_max")).toInt();
            ++n_params;
        }
        if (r->hasArg(F("R_min")))
        {
            prot_sensor->R_min = r->arg(F("R_min")).toInt();
            ++n_params;
        }
        if (r->hasArg(F("inverse_ADC")))
        {
            prot_sensor->inverse_ADC = (bool)(r->arg(F("inverse_ADC")).toInt());
            ++n_params;
        }
        // Calibration Step I
        if (r->hasArg(F("ADC_max")) && r->hasArg(F("ADC_min")) && r->hasArg(F("ADC_zero"))) // Step I
        {
            int ADC_max = r->arg(F("ADC_max")).toInt();
            int ADC_min = r->arg(F("ADC_min")).toInt();
            int ADC_zero = r->arg(F("ADC_zero")).toInt();
            p_cfg->body.ADC_min = ADC_min;
            p_cfg->body.ADC_max = ADC_max;
            p_cfg->body.ADC_zero = ADC_zero;
            prot_sensor->set_ADC_range(ADC_min, ADC_max, ADC_zero);
        }
        // Calibration Step 2
        if (r->hasArg(F("CW_lim")) && r->hasArg(F("CCW_lim"))) // Step 2
        {
            int CW_lim = r->arg(F("CW_lim")).toInt();
            int CCW_lim = r->arg(F("CCW_lim")).toInt();
            p_cfg->body.deg_limit_CW = CW_lim;
            p_cfg->body.deg_limit_CCW = CCW_lim;
            prot_sensor->deg_limit_F = CW_lim;
            prot_sensor->deg_limit_B = CCW_lim;
        }
    }
    else if (r->method() == HTTP_GET)
    {
        /* code */
        if (r->hasArg(F("manual"))) // manual calibration
        {
            is_calibrating = true;
        }
        else if (r->hasArg(F("auto")))
        {
            if ((!is_calibrating) && pmotor->status == MOT_IDLE)
            {
                begin_auto_calibrate();
            }
            else
            {
                r->send(300, F("system is busy"));
                return;
            }
        }
        else if (r->hasArg(F("stop")))
        {
            is_calibrating = false;
            is_auto_calibrating = false;
            cal_edge_count = 0;
            task_slot.type = NULL_TASK;
        }
        else if (r->hasArg(F("CLR_rounds"))) // clear n_rounds = 0
        {
            prot_sensor->n_rounds = 0;
        }
    }
    handleFileRead(r, "/config.html");
}

// respond with configurations
void onGetConfig(AsyncWebServerRequest *r)
{
    if (!is_config)
    {
        sprintf(s_httpbuffer, PSTR("is_calib=%d\n\n"), (int)is_calibrating);
    }
    else
    {
        sprintf(s_httpbuffer, PSTR("name=%s&ssid=%s&ip=%s&rssi=%d&pot_type=%d&multi_rounds=%d&break_delay=%d&R_0=%d&R_c=%d&R_max=%d&R_min=%d&ADC_min=%d&ADC_max=%d&ADC_zero=%d&inverse_ADC=%d&degree=%d&CW_lim=%d&CCW_lim=%d&is_calib=%d\n\n"),
                p_cfg->body.s_name,
                p_cfg->body.s_ssid,
                s_ip.c_str(),
                WiFi.RSSI(),
                (int)prot_sensor->pot_type,
                (int)prot_sensor->allow_multi_rounds,
                p_cfg->body.break_engage_defer,
                prot_sensor->R_0,
                prot_sensor->R_c,
                prot_sensor->R_max,
                prot_sensor->R_min,
                prot_sensor->ADC_min,
                prot_sensor->ADC_max,
                prot_sensor->ADC_zero,
                (int)prot_sensor->inverse_ADC,
                (int)(prot_sensor->degree),
                prot_sensor->deg_limit_F,
                prot_sensor->deg_limit_B,
                (int)is_calibrating);
    }
    r->send(200, "text/plain", s_httpbuffer);
}
char *sensorData()
{
    sprintf(s_httpbuffer, PSTR("azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"),
            prot_sensor->get_degree(),
            prot_sensor->get_ADC(),
            ((pmotor->status == MOT_IDLE) ? (0) : (1)),
            prot_sensor->is_limit(),
            prot_sensor->n_deg_speed);
    return s_httpbuffer;
}

//
void onGetWiFiStatus(AsyncWebServerRequest *r)
{
    String message;
    message += "WiFi_mode=";
    message += WiFi.getMode();
    message += "\nSSID=";
    message += WiFi.SSID();
    message += "\nPassword=";
    message += p_cfg->body.s_password;
    message += "\nIP=";
    message += s_ip;
    message += "\nRSSI=";
    message += WiFi.RSSI();
    message += "\n";
    r->send(200, "text/plain", message);
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
    if (r->hasArg(F("type")) && r->hasArg("to") && r->hasArg(F("speed")))
    {
        float deg = prot_sensor->degree;
        int type = r->arg(F("type")).toInt();
        int to = r->arg("to").toInt();
        int to_2, to_3;
        int speed = r->arg(F("speed")).toInt();
        // create a task object
        if ((speed >= 0 && speed <= 255) && (type == MANUAL || type == TARGET || type == NULL_TASK))
        {
            if (type == MANUAL &&
                to <= MANUAL_INTERVAL &&
                to >= -MANUAL_INTERVAL) // Task Type 1: Manual control
            {
                // create a manual control task object
                task_slot.type = MANUAL;
                task_slot.n_to = to;
                task_slot.n_speed = speed;
                task_slot.is_executed = false;
                LOG_DEBUG("Manual task recieved to %d\n", to);
            }
            else if (type == TARGET && to >= 0 && to <= 360) // Task Type 2: To a target
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
                LOG_DEBUG("Target task recieved to %d\n", task_slot.n_to);
            }
            else if (type == NULL_TASK)
            {
                task_slot.type = NULL_TASK;
                task_slot.is_executed = false;
                task_slot.n_to = 0;
            }
            r->send(200, "text/plain", F("task created\n"));
        }
        else
        {
            r->send(400, "text/plain", F("invalid parameters\n"));
        }
    }
    r->send(400, "text/plain", ("Bad request\n"));
}

void onHomepage(AsyncWebServerRequest *r)
{
    if (is_config)
    {
        handleFileRead(r, ("/index.html"));
    }
    else
    {
        handleFileRead(r, ("/config.html"));
    }
}

void onDebug485(AsyncWebServerRequest *r)
{
    handleFileRead(r, "/index.html");
}

void onWebConfig(AsyncWebServerRequest *r)
{
    handleFileRead(r, "/config.html");
}

void onStatus(AsyncWebServerRequest *r)
{
    sprintf(s_httpbuffer,
            PSTR("Free Memory=%d\nWiFi mode=%d\nRSSI=%d\n\n"),
            ESP.getFreeHeap(), WiFi.getMode(), WiFi.RSSI());
    r->send(200, "text/plain", s_httpbuffer);
}

// get content in RS485 recv buffer
void onWebGetRS485(AsyncWebServerRequest *r)
{
    char s[COMM_BUFFER_SIZE + 1];
    if (p485->idx_command > 0)
    {
        size_t lenRead = (p485->idx_command < COMM_BUFFER_SIZE) ? p485->idx_command : COMM_BUFFER_SIZE;
        strncpy(s, p485->command, lenRead);
        s[lenRead + 1] = 0; // trailing 0
        r->send(200, "text/plain", s);
    }
}

// post data to 485 tx
// POST
// args: data=bytestring, type="text" or "hex", newline=(true|false)
void onWebPostRS485(AsyncWebServerRequest *r)
{
    char s[COMM_BUFFER_SIZE + 1];
    if (r->method() == HTTP_POST && r->hasArg(F("data")))
    {
        const String& sdata = r->arg(F("data"));
        uint16_t lenDataTake = (sdata.length() < COMM_BUFFER_SIZE)? sdata.length() : COMM_BUFFER_SIZE;
        uint16_t lenDataSend = 0;
        if(lenDataTake > 0)
        {
            // hex mode, data is a string of hex-format chars e.g. aa bb cc dd ee a0
            if (r->hasArg(F("hex")) && r->arg(F("hex")).equals("on"))
            {
                uint16_t i, j;
                j = 0;
                // remove white spaces
                for(i=0; i<lenDataTake; ++i)
                {
                    if((sdata[i] >= '0' && sdata[i] <= '9') || (sdata[i] >= 'a' && sdata[i] <= 'z') || (sdata[i] >= 'A' && sdata[i] <= 'Z')) 
                        s[j++] = sdata[i];
                }
                // conver hex chars to uint8 two by two
                for(i=0; i<lenDataTake; i+=2)
                {
                    uint8_t v  = hexchr2num(s[i]);
                    if(i+1 < lenDataTake)
                        v = (v << 4) | hexchr2num(s[i+1]);
                    // replace hex chars by uint8_t value
                    s[i / 2] = v;
                }
                // trailing 0
                lenDataSend = i / 2;
                s[lenDataSend] = 0;
                // write binary data directly
                p485->pSerial->write(s, lenDataSend);
            }
            else // text mode
            {
                strncpy(s, sdata.c_str(), lenDataTake);
                if(r->hasArg(F("newline")) && r->arg(F("newline")).equals("on"))
                {
                    if(s[lenDataTake-1] == 0)
                    {
                        s[lenDataTake-1] = '\r';
                        s[lenDataTake] = 0;
                        lenDataSend = lenDataTake;
                    }
                    else
                    {
                        s[lenDataTake] = '\r';
                        s[lenDataTake+1] = 0;
                        lenDataSend = lenDataTake + 1;
                    }
                }
                else
                {
                    s[lenDataTake] = 0;
                    lenDataSend = lenDataTake;

                }
                send_serial485(p485, s);
            }
        }
        r->send(200, "text/plain", String(lenDataSend));
    }
    else // nothing to send
        r->send(300, "text/plain", "0");
}

void onNotFound(AsyncWebServerRequest *r)
{
    if (!handleFileRead(r, r->url())) // if no file matches the request, return with ERROR 404
    {
        String message;
        message.reserve(512);
        message += (F("AsyncServer: File Not Found\n\n"));

        message += F("URI: ");
        if (r->url().length() < 128)
            message += r->url();
        else
            message += F("TOO LONG");
        message += F("\nMethod: ");
        message += (r->method() == HTTP_GET) ? F("GET") : F("POST");
        message += F("\nArguments: ");
        message += r->args();
        message += F("\n");
        for (uint8_t i = 0; (i < r->args()) && (i < 8); i++)
        {
            if (r->argName(i).length() <= 16 && r->arg(i).length() <= 16)
                message += " " + r->argName(i) + ": " + r->arg(i) + "\n";
            else
                message += F("ARGUMENT TOO LONG");
        }
        r->send(404, ("text/plain"), message);
    }
}

// Save config and reset
void onReset(AsyncWebServerRequest *r)
{
    r->send(200, F("text/plain"), F("Restarting...\n"));
    // Serial.println("Reset...");
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
        prot_sensor->on_edge = NULL;
    }
}

void onCalibrate(AsyncWebServerRequest *r)
{
    if (r->method() == HTTP_POST)
    {
        String message(F("ADC calibrate:"));
        if (r->hasArg(F("ADC_max")) && r->hasArg(F("ADC_min")) && r->hasArg(F("ADC_zero"))) // Step I
        {
            int ADC_max = r->arg(F("ADC_max")).toInt();
            int ADC_min = r->arg(F("ADC_min")).toInt();
            int ADC_zero = r->arg(F("ADC_zero")).toInt();
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
        if (r->hasArg(F("CW_lim")) && r->hasArg(F("CCW_lim"))) // Step 2
        {
            int CW_lim = r->arg(F("CW_lim")).toInt();
            int CCW_lim = r->arg(F("CCW_lim")).toInt();
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
    else if (r->hasArg(F("manual"))) // manual calibration mode
    {
        is_calibrating = true;
        r->redirect("/calibrate");
    }
    else if (r->hasArg(F("auto"))) // GET request to initialize calibration
    {
        if ((!is_calibrating) && pmotor->status == MOT_IDLE)
        {
            begin_auto_calibrate();
            r->send(200, F("Auto calibration started\n"));
        }
        else
        {
            r->send(300, "system is busy");
        }
    }
    else if (r->hasArg(F("stop")))
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
        handleFileRead(r, "/calibrate.html");
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
    ++tick;
    // TASK: BLINK LED
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

    /***********************
     *  AP connection bounday
     * below will not be executed if AP connection is needed but 0 client has connected
     ***********************/
    yield();
    if (!p485->is_active && (WiFi.getMode() == WIFI_AP) && (WiFi.softAPgetStationNum() == 0)) // waiting for AP connections if 485 is not connected.
        return;

    // TASK: Autosave EEPROM
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

    // TASK: send web data 10 / sec
    if (tick % (TIMER_PER_SEC / 5) == 1 && webSocket.count() > 0 )
    {
        if(webSocket.availableForWriteAll())
            webSocket.textAll(sensorData());
        else
            webSocket.closeAll();
    }
    // TASK: update ADC
    prot_sensor->append_ADC_reading(ADC_smoothed);
    // TASK: Calibrating ADC
    if (is_auto_calibrating)
    {
        int n = prot_sensor->ADC_reading;
        if (n > prot_sensor->ADC_max)
            prot_sensor->ADC_max = n;
        else if (n < prot_sensor->ADC_min)
            prot_sensor->ADC_min = n;
    }
    int deg_now = prot_sensor->get_degree();
    // TASK: toggle limit protection
    if ((!is_calibrating) && (pmotor->status == prot_sensor->is_limit() != 0) && (prot_sensor->is_limit() != 0))
    {
        // stop the task
        LOG_DEBUG("LOG: Limit triggered, attempt to stop\n");
        webSocket.textAll("LOG: Limit triggered.");
        if (stop_motor(pmotor))
        {
            pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
            task_slot.type = NULL_TASK;
        }
    }
    // TASK: timeout limit protection each 3 sec
    if ((tick % TIMER_PER_SEC == 2) &&
        (pmotor->status != MOT_IDLE) &&
        task_slot.type != NULL_TASK &&
        (is_calibrating == is_auto_calibrating)) // time limit tiggers either not calibrating or auto calibrating
    {
        if (abs(prot_sensor->n_deg_speed) <= 3 || SGN(prot_sensor->n_deg_speed) != pmotor->status)
        {

            // timeout limit protection triggered
            if (++(prot_sensor->stat_limit_T) >= 4)
            {
                prot_sensor->stat_limit_T = 0;
                LOG_DEBUG("LOG: Time limit triggered, attempt to stop\n");
                webSocket.textAll("LOG: Time limit triggered.");
                while (!stop_motor(pmotor))
                    ;
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
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
    // TASK: judge if the task is completed
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
            while (!stop_motor(pmotor))
                ;
            pmotor->break_engage_defer = p_cfg->body.break_engage_defer; // set break engage timeout flag
            LOG_DEBUG("Task completed\n");
            return;
        }
    }
    else if (task_slot.type == TARGET)
    {
        if ((pmotor->status == MOT_CW && deg_now >= task_slot.n_to) || (pmotor->status == MOT_CCW && deg_now <= task_slot.n_to))
        {
            if (stop_motor(pmotor)) // return true if the motor stopping signial is confirmed
            {
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                task_slot.type = NULL_TASK;
                LOG_DEBUG("Task completed\n");
                return;
            }
        }
    }
    accelerate(pmotor);
    // TASK: handle break engagement waiting time (longer waiting time for heavier antenna sets)
    if (pmotor->break_engage_defer > 0)
    {
        pmotor->break_engage_defer -= (SCHED_INTERVAL / 1000);
        if (pmotor->break_engage_defer <= 0)
        {
            pmotor->break_engage_defer = 0;
            engage_break(pmotor);
        }
    }
    // TASK: execute the task
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
                    LOG_DEBUG("Manual task started\n");
                }
                else // kill the task if limit switches are toggled
                {
                    if (stop_motor(pmotor))
                    {
                        pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                        LOG_DEBUG("Task killed for limits\n");
                        task_slot.type = NULL_TASK;
                    }
                }
                break;
            case TARGET:
                if (prot_sensor->is_ADC_calibrated)
                {
                    // CW CCW
                    if (task_slot.n_to > deg_now + MOT_DEADZONE || task_slot.n_to < deg_now - MOT_DEADZONE)
                    {
                        run_CW(pmotor);
                        LOG_DEBUG("Target task created\n");
                    }
                    // DEADZONE
                    else
                    {
                        task_slot.type = NULL_TASK;
                        LOG_DEBUG("Task killed for deadzone\n");
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
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                task_slot.type = NULL_TASK;
                LOG_DEBUG("LOG: task cancelled\n");
            }
        }
    }
}

// OTA init
void init_OTA()
{
    ArduinoOTA.onStart([]()
                       {
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
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf_P(PSTR("Progress: %u%%\r"), (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
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
        // os_printf("WS connected %d\n", server->count());
        client->keepAlivePeriod(90);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        // os_printf("WS disconnected %d\n", client->id());
    }
    else if (type == WS_EVT_PONG)
    {
        // os_printf("WS PONG %d\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_PING) // heart beat from the client
        {
        }
        else if (info->opcode == WS_TEXT)
        {
            // os_printf("WS recved: %s\n", (char *)data);
        }
        else
        {

            // os_printf("WS recved\n");
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
    is_config = true;
    is_wifi_conn = false;
    mot_dir = 0;
    mot_pwm = 0;
    time_unsaved_ms = 0;
    //************** initializing objects **************//
    // SPIFFS
    SPIFFS.begin();
    // 485
    begin_serial485(p485, &Serial, BAUD_RATE, PIN_485RW, SCHED_INTERVAL / 1000);
#ifdef __DEBUG_
    gdbstub_init();
#endif
    //595
    begin_595(p595, PIN_SCLK, PIN_RCLK, PIN_OE, PIN_DATA);
    // EEPROM
    begin_AT24C(pEEPROM, AT24C64, PIN_SCL, PIN_SDA, PIN_WP, 0);
    // task slot
    init_task(&task_slot);
    // motor

    init_motor(pmotor, PIN_BREAK, PIN_MOTOR_N, PIN_MOTOR_P);
    //************* read config from EEPROM ****************//
    // initialize config/param datastructures
    load_config(p_cfg);
    load_active_params(p_actprm);
    // Serial.printf("ID: %s\n", p_cfg->body.s_id);
    if (strcmp(p_cfg->body.s_id, _ID) == 0) // check if config is valid
    {
        //Serial.println("Configuration found!");
        /** configuration found  **/
        // load ADC settings
        // read sensors to get ADC now
        unsigned int ADC_now = 0;
        unsigned int ADC_stored = p_actprm->body.ADC_reading;
        // obtain initial readings
        while (!(prot_sensor->ADC_results->is_full() && prot_sensor->deg_results->is_full()))
        {
            prot_sensor->append_ADC_reading(analogRead(A0));
            delay(TIMER_INTERVAL / 1000);
        }
        prot_sensor->is_stable = true;
        ADC_smoothed = (float)analogRead(A0);

        // Use loaded config to initialize the sensor
        prot_sensor->pot_type = (enum PotType)p_cfg->body.pot_type;
        prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
        prot_sensor->ADC_max = p_cfg->body.ADC_max;
        prot_sensor->ADC_min = p_cfg->body.ADC_min;
        prot_sensor->ADC_zero = p_cfg->body.ADC_zero;
        prot_sensor->inverse_ADC = p_cfg->body.inverse_ADC;
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
        if (true)
        {
            // Serial.printf("Connecting STA %s\r\n", p_cfg->body.s_ssid);
            WiFi.mode(WIFI_STA);
            delay(100);
            WiFi.setOutputPower(20.5); // WiFi set to max TX power
            WiFi.begin(p_cfg->body.s_ssid, p_cfg->body.s_password);
            int n_conn_try = 0;
            while ((WiFi.status() != WL_CONNECTED) && (n_conn_try++ < 5))
            {
                delay(1000);
            }
            if (WiFi.status() == WL_CONNECTED)
            {
                s_ip = WiFi.localIP().toString();
                LOG_DEBUG("WiFi connected  ");
                LOG_DEBUG("IP=%s\n", s_ip.c_str());
                is_config = true;
                is_wifi_conn = true;
                // Turn on the LED
                stat_LED_stat = 1;
                TURN_ON_LED(PIN_LED_STAT);
            }
            else // falied connecting to WiFi
            {
                // Serial.println("STA connection failed");
                WiFi.disconnect(true);
                delay(500);
            }
        }
    }
    else // not correctly configured. Initialize an Acess Point for configuration
    {
        LOG_DEBUG("check sign mismatch");
        is_config = false;
        prot_sensor->is_ADC_calibrated = false;
        // empty the uninitialized strings
        p_cfg->body.s_id[0] = 0;
        p_cfg->body.s_name[0] = 0;
        p_cfg->body.s_password[0] = 0;
        p_cfg->body.s_ssid[0] = 0;
        p_cfg->body.is_ADC_calibrated = false;
        p_cfg->body.break_engage_defer = 1000; // by default 1000ms
    }

    ESP.wdtEnable(2000);
    // if falied to connect WiFi as STA, initiate an AP hotspot
    if (!WiFi.isConnected())
    {
        is_config = false;
        WiFi.mode(WIFI_AP);
        if (WiFi.softAP(("VORTEX1")))
        {
            delay(100);
            WiFi.setOutputPower(20.5); // WiFi set to max TX power
            WiFi.softAPConfig(AP_IP, gateway, subnet);
            LOG_DEBUG("AP started");
            s_ip = WiFi.softAPIP().toString();
            stat_LED_stat = 4;
            delay(2000);
        }
    }

    // initialize HTTP service
    server.on("/", HTTP_GET, onHomepage);
    server.on("/calibrate", onCalibrate);
    server.on("/setconfig", onSetConfig);
    server.on("/getconfig", onGetConfig);
    server.on("/config", onWebConfig);
    server.on("/reset", onReset);
    server.on("/getSensor", onGetSensor);
    server.on("/wifiStatus", onGetWiFiStatus);
    server.on("/task", HTTP_POST, onTask);
    server.on("/rs485", onDebug485);
    server.on(("/status"), onStatus);
    // RS485 test pages
    server.on("/getRS485", HTTP_GET, onWebGetRS485);    // get RS485 recieve buffer
    server.on("/postRS485", HTTP_POST, onWebPostRS485); // post data to RS485
    server.onNotFound(onNotFound);
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
    // Serial.print(s_ip);
}
#define ADC_SMOOTHING_PERIOD 16
#define ADC_SMOOTHING_CONST (2.0 / (ADC_SMOOTHING_PERIOD + 1))
void loop()
{
    static unsigned int ticker = micros();
    static unsigned int ticker_ADC = ticker; // a 2ms ticker for ADC smoothing
    unsigned long ticker_new = micros();
    unsigned long ticker_ADC_new = ticker_new;
    // using EMA to smooth ADC readings
    if (ticker_ADC_new - ticker >= 50000) // smooth the ADC readings
    {
        int ADC_reading = analogRead(A0);
        ticker_ADC = ticker_ADC_new;
        ADC_smoothed = (ADC_reading - ADC_smoothed) * ADC_SMOOTHING_CONST + ADC_smoothed;
    }
    if (ticker_new - ticker >= SCHED_INTERVAL)
    {
        ticker = ticker_new;
        // TASK: handle 485
        handle_serial485(p485);
        onTimer();
    }
    // handle OTA
    ArduinoOTA.handle();
}
// TODO: 485

