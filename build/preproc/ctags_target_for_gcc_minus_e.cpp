# 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"




//#define __DEBUG_
// TIMER FREQUENCY = CPU FREQ / 16

# 9 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 10 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 11 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 12 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 13 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 14 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 15 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 16 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 17 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2




# 22 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 23 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 24 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 25 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 26 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 27 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 28 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2

// SPIFFS is deprecated, move to LittleFS since ESP8266 Arduino 3.0.2


# 33 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 34 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 42 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
// pin number of 485 Read/write switch
# 51 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
// Limit Switch Input


// 74595





// AT24C64




// LEDs



// Relays





// Motor




// sensor


// Sched Intervals




// Network



IPAddress AP_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
# 110 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
// global variables
byte stat_LED_stat = 0; // 0: OFF, 255: ON, 1-254 BLINKS PER SEC
bool level_led_stat = 0x1;
byte stat_LED_task = 0; // 0: OFF, 255: ON, 1-254 BLINKS PER SEC
bool level_led_task = 0x1;
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
RotSensor rot_sensor(TWO_TERMINALS, 50000 /* us*/);
RotSensor *prot_sensor = &rot_sensor;
// create motor object
struct Motor motor;
struct Motor *pmotor = &motor;
// create web server objects
//ESP8266WebServer server(HTTP_PORT);
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
//WebSocketsServer webSocket = WebSocketsServer(WS_PORT);

/********************************************
 *         handle  EEPROM                   *
*/
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
    if (path.endsWith(((reinterpret_cast<const __FlashStringHelper *>(
# 212 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "212" "." "15" "\", \"aSM\", @progbits, 1 #"))) = (
# 212 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "/"
# 212 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 212 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
    {
        path += ((reinterpret_cast<const __FlashStringHelper *>(
# 214 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
               (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "214" "." "16" "\", \"aSM\", @progbits, 1 #"))) = (
# 214 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
               "index.html"
# 214 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
               ); &__pstr__[0];}))
# 214 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
               )));
    }

    String contentType = getContentType(path);
    String path_gz(path);
    path_gz += (".gz");
    if (LittleFS.exists(path_gz))
    {
        AsyncWebServerResponse *response = r->beginResponse(LittleFS, path_gz, contentType);
        response->addHeader(("Content-Encoding"), ("gzip"));
        r->send(response);
        return true;
    }
    else if (LittleFS.exists(path))
    {
        r->send(LittleFS, path, contentType);
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
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 241 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "241" "." "17" "\", \"aSM\", @progbits, 1 #"))) = (
# 241 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "name"
# 241 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 241 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            strcpy(p_cfg->body.s_name, r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 243 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "243" "." "18" "\", \"aSM\", @progbits, 1 #"))) = (
# 243 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             "name"
# 243 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             ); &__pstr__[0];}))
# 243 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             )))).c_str());
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 247 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "247" "." "19" "\", \"aSM\", @progbits, 1 #"))) = (
# 247 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "ssid"
# 247 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 247 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            strcpy(p_cfg->body.s_ssid, r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 249 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "249" "." "20" "\", \"aSM\", @progbits, 1 #"))) = (
# 249 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             "ssid"
# 249 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             ); &__pstr__[0];}))
# 249 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             )))).c_str());
            // Serial.printf("ssid=%s\r\n", p_cfg->body.s_ssid);
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 254 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "254" "." "21" "\", \"aSM\", @progbits, 1 #"))) = (
# 254 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "passwd"
# 254 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 254 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            strcpy(p_cfg->body.s_password, r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 256 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                 (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "256" "." "22" "\", \"aSM\", @progbits, 1 #"))) = (
# 256 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                 "passwd"
# 256 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                 ); &__pstr__[0];}))
# 256 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                 )))).c_str());
            ++n_params;
            strcpy(p_cfg->body.s_id, "VORTEX2");
            p_cfg->body.is_WiFi_set = true;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 262 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "262" "." "23" "\", \"aSM\", @progbits, 1 #"))) = (
# 262 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "pot_type"
# 262 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 262 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            p_cfg->body.pot_type = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 264 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                         (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "264" "." "24" "\", \"aSM\", @progbits, 1 #"))) = (
# 264 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                         "pot_type"
# 264 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                         ); &__pstr__[0];}))
# 264 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                         )))).toInt();
            prot_sensor->pot_type = (PotType)p_cfg->body.pot_type;
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 268 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "268" "." "25" "\", \"aSM\", @progbits, 1 #"))) = (
# 268 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "multi_rounds"
# 268 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 268 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            p_cfg->body.allow_multi_rounds = (r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 270 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "270" "." "26" "\", \"aSM\", @progbits, 1 #"))) = (
# 270 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    "multi_rounds"
# 270 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    ); &__pstr__[0];}))
# 270 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    )))).toInt() > 0);
            prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 274 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "274" "." "27" "\", \"aSM\", @progbits, 1 #"))) = (
# 274 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "break_delay"
# 274 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 274 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            p_cfg->body.break_engage_defer = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 276 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                   (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "276" "." "28" "\", \"aSM\", @progbits, 1 #"))) = (
# 276 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                   "break_delay"
# 276 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                   ); &__pstr__[0];}))
# 276 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                   )))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 279 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "279" "." "29" "\", \"aSM\", @progbits, 1 #"))) = (
# 279 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "R_0"
# 279 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 279 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            prot_sensor->R_0 = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 281 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "281" "." "30" "\", \"aSM\", @progbits, 1 #"))) = (
# 281 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                     "R_0"
# 281 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                     ); &__pstr__[0];}))
# 281 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                     )))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 284 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "284" "." "31" "\", \"aSM\", @progbits, 1 #"))) = (
# 284 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "R_c"
# 284 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 284 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            prot_sensor->R_c = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 286 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "286" "." "32" "\", \"aSM\", @progbits, 1 #"))) = (
# 286 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                     "R_c"
# 286 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                     ); &__pstr__[0];}))
# 286 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                     )))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 289 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "289" "." "33" "\", \"aSM\", @progbits, 1 #"))) = (
# 289 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "R_max"
# 289 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 289 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            prot_sensor->R_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 291 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                       (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "291" "." "34" "\", \"aSM\", @progbits, 1 #"))) = (
# 291 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                       "R_max"
# 291 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                       ); &__pstr__[0];}))
# 291 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                       )))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 294 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "294" "." "35" "\", \"aSM\", @progbits, 1 #"))) = (
# 294 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "R_min"
# 294 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 294 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            prot_sensor->R_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 296 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                       (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "296" "." "36" "\", \"aSM\", @progbits, 1 #"))) = (
# 296 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                       "R_min"
# 296 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                       ); &__pstr__[0];}))
# 296 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                       )))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 299 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "299" "." "37" "\", \"aSM\", @progbits, 1 #"))) = (
# 299 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "inverse_ADC"
# 299 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 299 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))))
        {
            prot_sensor->inverse_ADC = (bool)(r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 301 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "301" "." "38" "\", \"aSM\", @progbits, 1 #"))) = (
# 301 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    "inverse_ADC"
# 301 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    ); &__pstr__[0];}))
# 301 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    )))).toInt());
            ++n_params;
        }
        // Calibration Step I
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "305" "." "39" "\", \"aSM\", @progbits, 1 #"))) = (
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "ADC_max"
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "305" "." "40" "\", \"aSM\", @progbits, 1 #"))) = (
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                "ADC_min"
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                ); &__pstr__[0];}))
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                                           (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "305" "." "41" "\", \"aSM\", @progbits, 1 #"))) = (
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                           "ADC_zero"
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                                           ); &__pstr__[0];}))
# 305 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                           ))))) // Step I
        {
            int ADC_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 307 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "307" "." "42" "\", \"aSM\", @progbits, 1 #"))) = (
# 307 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "ADC_max"
# 307 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 307 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
            int ADC_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 308 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "308" "." "43" "\", \"aSM\", @progbits, 1 #"))) = (
# 308 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "ADC_min"
# 308 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 308 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
            int ADC_zero = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 309 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "309" "." "44" "\", \"aSM\", @progbits, 1 #"))) = (
# 309 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 "ADC_zero"
# 309 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 ); &__pstr__[0];}))
# 309 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 )))).toInt();
            p_cfg->body.ADC_min = ADC_min;
            p_cfg->body.ADC_max = ADC_max;
            p_cfg->body.ADC_zero = ADC_zero;
            prot_sensor->set_ADC_range(ADC_min, ADC_max, ADC_zero);
        }
        // Calibration Step 2
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "316" "." "45" "\", \"aSM\", @progbits, 1 #"))) = (
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "CW_lim"
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                               (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "316" "." "46" "\", \"aSM\", @progbits, 1 #"))) = (
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                               "CCW_lim"
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                               ); &__pstr__[0];}))
# 316 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                               ))))) // Step 2
        {
            int CW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 318 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                               (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "318" "." "47" "\", \"aSM\", @progbits, 1 #"))) = (
# 318 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                               "CW_lim"
# 318 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                               ); &__pstr__[0];}))
# 318 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                               )))).toInt();
            int CCW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 319 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "319" "." "48" "\", \"aSM\", @progbits, 1 #"))) = (
# 319 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "CCW_lim"
# 319 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 319 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
            p_cfg->body.deg_limit_CW = CW_lim;
            p_cfg->body.deg_limit_CCW = CCW_lim;
            prot_sensor->deg_limit_F = CW_lim;
            prot_sensor->deg_limit_B = CCW_lim;
        }
    }
    else if (r->method() == HTTP_GET)
    {
        /* code */
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 329 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "329" "." "49" "\", \"aSM\", @progbits, 1 #"))) = (
# 329 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "manual"
# 329 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 329 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     ))))) // manual calibration
        {
            is_calibrating = true;
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 333 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "333" "." "50" "\", \"aSM\", @progbits, 1 #"))) = (
# 333 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          "auto"
# 333 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          ); &__pstr__[0];}))
# 333 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          )))))
        {
            if ((!is_calibrating) && pmotor->status == 0)
            {
                begin_auto_calibrate();
            }
            else
            {
                r->send(300, ((reinterpret_cast<const __FlashStringHelper *>(
# 341 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                            (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "341" "." "51" "\", \"aSM\", @progbits, 1 #"))) = (
# 341 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                            "system is busy"
# 341 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                            ); &__pstr__[0];}))
# 341 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                            ))));
                return;
            }
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 345 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "345" "." "52" "\", \"aSM\", @progbits, 1 #"))) = (
# 345 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          "stop"
# 345 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          ); &__pstr__[0];}))
# 345 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          )))))
        {
            is_calibrating = false;
            is_auto_calibrating = false;
            cal_edge_count = 0;
            task_slot.type = 0;
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 352 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "352" "." "53" "\", \"aSM\", @progbits, 1 #"))) = (
# 352 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          "CLR_rounds"
# 352 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          ); &__pstr__[0];}))
# 352 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          ))))) // clear n_rounds = 0
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
        sprintf(s_httpbuffer, 
# 365 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "365" "." "54" "\", \"aSM\", @progbits, 1 #"))) = (
# 365 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                             "is_calib=%d\n\n"
# 365 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                             ); &__pstr__[0];}))
# 365 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    , (int)is_calibrating);
    }
    else
    {
        sprintf(s_httpbuffer, 
# 369 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "369" "." "55" "\", \"aSM\", @progbits, 1 #"))) = (
# 369 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                             "name=%s&ssid=%s&ip=%s&rssi=%d&pot_type=%d&multi_rounds=%d&break_delay=%d&R_0=%d&R_c=%d&R_max=%d&R_min=%d&ADC_min=%d&ADC_max=%d&ADC_zero=%d&inverse_ADC=%d&degree=%d&CW_lim=%d&CCW_lim=%d&is_calib=%d\n\n"
# 369 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                             ); &__pstr__[0];}))
# 369 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                                                                                                                                                                                             ,
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
    sprintf(s_httpbuffer, 
# 394 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "394" "." "56" "\", \"aSM\", @progbits, 1 #"))) = (
# 394 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                         "azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"
# 394 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         ); &__pstr__[0];}))
# 394 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                        ,
            prot_sensor->get_degree(),
            prot_sensor->get_ADC(),
            ((pmotor->status == 0) ? (0) : (1)),
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
    if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                 (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "433" "." "57" "\", \"aSM\", @progbits, 1 #"))) = (
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                 "type"
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                 ); &__pstr__[0];}))
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                 )))) && r->hasArg("to") && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                            (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "433" "." "58" "\", \"aSM\", @progbits, 1 #"))) = (
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                            "speed"
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                            ); &__pstr__[0];}))
# 433 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                            )))))
    {
        float deg = prot_sensor->degree;
        int type = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 436 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "436" "." "59" "\", \"aSM\", @progbits, 1 #"))) = (
# 436 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                         "type"
# 436 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         ); &__pstr__[0];}))
# 436 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                         )))).toInt();
        int to = r->arg("to").toInt();
        int to_2, to_3;
        int speed = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 439 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "439" "." "60" "\", \"aSM\", @progbits, 1 #"))) = (
# 439 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          "speed"
# 439 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          ); &__pstr__[0];}))
# 439 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          )))).toInt();
        // create a task object
        if ((speed >= 0 && speed <= 255) && (type == 1 || type == 2 || type == 0))
        {
            if (type == 1 &&
                to <= 75000 /* ms*/ &&
                to >= -75000 /* ms*/) // Task Type 1: Manual control
            {
                // create a manual control task object
                task_slot.type = 1;
                task_slot.n_to = to;
                task_slot.n_speed = speed;
                task_slot.is_executed = false;
                do { (void)0; } while (0);
            }
            else if (type == 2 && to >= 0 && to <= 360) // Task Type 2: To a target
            {
                task_slot.type = 2;
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
                    to_2 = (((((to_2 - deg) >= 0) ? ((to_2 - deg)) : (-(to_2 - deg))) < (((to_3 - deg) >= 0) ? ((to_3 - deg)) : (-(to_3 - deg)))) ? (to_2) : (to_3));
                }
                task_slot.n_to = (((((to_2 - deg) >= 0) ? ((to_2 - deg)) : (-(to_2 - deg))) < (((to - deg) >= 0) ? ((to - deg)) : (-(to - deg)))) ? (to_2) : (to));
                task_slot.is_executed = false;
                do { (void)0; } while (0);
            }
            else if (type == 0)
            {
                task_slot.type = 0;
                task_slot.is_executed = false;
                task_slot.n_to = 0;
            }
            r->send(200, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>(
# 504 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "504" "." "61" "\", \"aSM\", @progbits, 1 #"))) = (
# 504 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                      "task created\n"
# 504 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                      ); &__pstr__[0];}))
# 504 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                      ))));
        }
        else
        {
            r->send(400, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>(
# 508 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "508" "." "62" "\", \"aSM\", @progbits, 1 #"))) = (
# 508 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                      "invalid parameters\n"
# 508 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                      ); &__pstr__[0];}))
# 508 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                      ))));
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
            
# 539 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
           (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "539" "." "63" "\", \"aSM\", @progbits, 1 #"))) = (
# 539 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
           "Free Memory=%d\nWiFi mode=%d\nRSSI=%d\n\n"
# 539 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
           ); &__pstr__[0];}))
# 539 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                            ,
            ESP.getFreeHeap(), WiFi.getMode(), WiFi.RSSI());
    r->send(200, "text/plain", s_httpbuffer);
}

// get content in RS485 recv buffer
void onWebGetRS485(AsyncWebServerRequest *r)
{
    char s[64 + 1];
    if (p485->idx_command > 0)
    {
        size_t lenRead = (p485->idx_command < 64) ? p485->idx_command : 64;
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
    char s[64 + 1];
    if (r->method() == HTTP_POST && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 563 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "563" "." "64" "\", \"aSM\", @progbits, 1 #"))) = (
# 563 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             "data"
# 563 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             ); &__pstr__[0];}))
# 563 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             )))))
    {
        const String& sdata = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 565 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "565" "." "65" "\", \"aSM\", @progbits, 1 #"))) = (
# 565 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                    "data"
# 565 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                    ); &__pstr__[0];}))
# 565 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                    ))));
        uint16_t lenDataTake = (sdata.length() < 64)? sdata.length() : 64;
        uint16_t lenDataSend = 0;
        if(lenDataTake > 0)
        {
            // hex mode, data is a string of hex-format chars e.g. aa bb cc dd ee a0
            if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "571" "." "66" "\", \"aSM\", @progbits, 1 #"))) = (
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                         "hex"
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                         ); &__pstr__[0];}))
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                         )))) && r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "571" "." "67" "\", \"aSM\", @progbits, 1 #"))) = (
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             "hex"
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                             ); &__pstr__[0];}))
# 571 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                             )))).equals("on"))
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
                    uint8_t v = hexchr2num(s[i]);
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
                if(r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                            (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "599" "." "68" "\", \"aSM\", @progbits, 1 #"))) = (
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                            "newline"
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                            ); &__pstr__[0];}))
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                            )))) && r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "599" "." "69" "\", \"aSM\", @progbits, 1 #"))) = (
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    "newline"
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                    ); &__pstr__[0];}))
# 599 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                    )))).equals("on"))
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
        message += (((reinterpret_cast<const __FlashStringHelper *>(
# 635 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                   (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "635" "." "70" "\", \"aSM\", @progbits, 1 #"))) = (
# 635 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                   "AsyncServer: File Not Found\n\n"
# 635 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                   ); &__pstr__[0];}))
# 635 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                   ))));

        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 637 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "637" "." "71" "\", \"aSM\", @progbits, 1 #"))) = (
# 637 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  "URI: "
# 637 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  ); &__pstr__[0];}))
# 637 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  )));
        if (r->url().length() < 128)
            message += r->url();
        else
            message += ((reinterpret_cast<const __FlashStringHelper *>(
# 641 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "641" "." "72" "\", \"aSM\", @progbits, 1 #"))) = (
# 641 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      "TOO LONG"
# 641 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      ); &__pstr__[0];}))
# 641 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      )));
        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 642 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "642" "." "73" "\", \"aSM\", @progbits, 1 #"))) = (
# 642 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  "\nMethod: "
# 642 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  ); &__pstr__[0];}))
# 642 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  )));
        message += (r->method() == HTTP_GET) ? ((reinterpret_cast<const __FlashStringHelper *>(
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "643" "." "74" "\", \"aSM\", @progbits, 1 #"))) = (
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                              "GET"
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                              ); &__pstr__[0];}))
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                              ))) : ((reinterpret_cast<const __FlashStringHelper *>(
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                         (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "643" "." "75" "\", \"aSM\", @progbits, 1 #"))) = (
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                         "POST"
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                         ); &__pstr__[0];}))
# 643 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                         )));
        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 644 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "644" "." "76" "\", \"aSM\", @progbits, 1 #"))) = (
# 644 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  "\nArguments: "
# 644 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  ); &__pstr__[0];}))
# 644 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  )));
        message += r->args();
        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 646 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "646" "." "77" "\", \"aSM\", @progbits, 1 #"))) = (
# 646 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  "\n"
# 646 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                  ); &__pstr__[0];}))
# 646 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                  )));
        for (uint8_t i = 0; (i < r->args()) && (i < 8); i++)
        {
            if (r->argName(i).length() <= 16 && r->arg(i).length() <= 16)
                message += " " + r->argName(i) + ": " + r->arg(i) + "\n";
            else
                message += ((reinterpret_cast<const __FlashStringHelper *>(
# 652 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "652" "." "78" "\", \"aSM\", @progbits, 1 #"))) = (
# 652 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          "ARGUMENT TOO LONG"
# 652 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                          ); &__pstr__[0];}))
# 652 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                          )));
        }
        r->send(404, ("text/plain"), message);
    }
}

// Save config and reset
void onReset(AsyncWebServerRequest *r)
{
    r->send(200, ((reinterpret_cast<const __FlashStringHelper *>(
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "661" "." "79" "\", \"aSM\", @progbits, 1 #"))) = (
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                "text/plain"
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                ); &__pstr__[0];}))
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                ))), ((reinterpret_cast<const __FlashStringHelper *>(
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "661" "." "80" "\", \"aSM\", @progbits, 1 #"))) = (
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 "Restarting...\n"
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 ); &__pstr__[0];}))
# 661 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 ))));
    // Serial.println("Reset...");
    sync_config(p_cfg);
    sync_active_params(p_actprm);
    save_config(p_cfg);
    save_active_params(p_actprm);
    LittleFS.end();
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
        task_slot.type = 0; // stop the motor
        prot_sensor->on_edge = 
# 684 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3 4
                              __null
# 684 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                  ;
    }
}

void onCalibrate(AsyncWebServerRequest *r)
{
    if (r->method() == HTTP_POST)
    {
        String message(((reinterpret_cast<const __FlashStringHelper *>(
# 692 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "692" "." "81" "\", \"aSM\", @progbits, 1 #"))) = (
# 692 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      "ADC calibrate:"
# 692 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      ); &__pstr__[0];}))
# 692 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      ))));
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "693" "." "82" "\", \"aSM\", @progbits, 1 #"))) = (
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "ADC_max"
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "693" "." "83" "\", \"aSM\", @progbits, 1 #"))) = (
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                "ADC_min"
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                ); &__pstr__[0];}))
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                                           (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "693" "." "84" "\", \"aSM\", @progbits, 1 #"))) = (
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                           "ADC_zero"
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                                                           ); &__pstr__[0];}))
# 693 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                           ))))) // Step I
        {
            int ADC_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 695 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "695" "." "85" "\", \"aSM\", @progbits, 1 #"))) = (
# 695 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "ADC_max"
# 695 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 695 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
            int ADC_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 696 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "696" "." "86" "\", \"aSM\", @progbits, 1 #"))) = (
# 696 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "ADC_min"
# 696 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 696 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
            int ADC_zero = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 697 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "697" "." "87" "\", \"aSM\", @progbits, 1 #"))) = (
# 697 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 "ADC_zero"
# 697 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                 ); &__pstr__[0];}))
# 697 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                 )))).toInt();
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
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "717" "." "88" "\", \"aSM\", @progbits, 1 #"))) = (
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     "CW_lim"
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                     ); &__pstr__[0];}))
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                     )))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                               (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "717" "." "89" "\", \"aSM\", @progbits, 1 #"))) = (
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                               "CCW_lim"
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                               ); &__pstr__[0];}))
# 717 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                               ))))) // Step 2
        {
            int CW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 719 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                               (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "719" "." "90" "\", \"aSM\", @progbits, 1 #"))) = (
# 719 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                               "CW_lim"
# 719 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                               ); &__pstr__[0];}))
# 719 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                               )))).toInt();
            int CCW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>(
# 720 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "720" "." "91" "\", \"aSM\", @progbits, 1 #"))) = (
# 720 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                "CCW_lim"
# 720 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                ); &__pstr__[0];}))
# 720 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                )))).toInt();
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
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 733 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "733" "." "92" "\", \"aSM\", @progbits, 1 #"))) = (
# 733 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      "manual"
# 733 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      ); &__pstr__[0];}))
# 733 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      ))))) // manual calibration mode
    {
        is_calibrating = true;
        r->redirect("/calibrate");
    }
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 738 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "738" "." "93" "\", \"aSM\", @progbits, 1 #"))) = (
# 738 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      "auto"
# 738 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      ); &__pstr__[0];}))
# 738 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      ))))) // GET request to initialize calibration
    {
        if ((!is_calibrating) && pmotor->status == 0)
        {
            begin_auto_calibrate();
            r->send(200, ((reinterpret_cast<const __FlashStringHelper *>(
# 743 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                        (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "743" "." "94" "\", \"aSM\", @progbits, 1 #"))) = (
# 743 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                        "Auto calibration started\n"
# 743 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                        ); &__pstr__[0];}))
# 743 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                        ))));
        }
        else
        {
            r->send(300, "system is busy");
        }
    }
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>(
# 750 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "750" "." "95" "\", \"aSM\", @progbits, 1 #"))) = (
# 750 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      "stop"
# 750 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                      ); &__pstr__[0];}))
# 750 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                      )))))
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
    set_task(&task_slot, 1, 0x7ffffff, 255);
}

// motor turns around when it runs to the limit
void auto_calibration_turn_around()
{
    // 0CW,1CCW,2CW,3CCW->ORIGIN
    ++n_auto_calibrate_rounds;
    switch (n_auto_calibrate_rounds)
    {
    case 1: // CCW
        set_task(&task_slot, 1, -0x7ffffff, 255);
        break;
    case 2: //CW
        set_task(&task_slot, 1, 0x7ffffff, 255);
        break;
    case 3: //CCW
        prot_sensor->update_ADC();
        prot_sensor->deg_limit_F = prot_sensor->get_deg_now();
        set_task(&task_slot, 1, -0x7ffffff, 255);
        break;
    case 4: // FINISHED, back to origin
        prot_sensor->deg_limit_B = prot_sensor->get_deg_now();
        set_task(&task_slot, 2, 0, 255);
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
    static unsigned char flash_LED = 0x0;
    unsigned long time_s = micros();
    ESP.wdtFeed();
    ++tick;
    // TASK: BLINK LED
    switch (stat_LED_stat)
    {
    case 0:
        (((((((1) | 64)) >= 64))?(write_595(p595, (((1) | 64)) - 64, 0x1)):(digitalWrite((((1) | 64)), (0x1)))));
        break;
    case 255:
        (((((((1) | 64)) >= 64))?(write_595(p595, (((1) | 64)) - 64, 0x0)):(digitalWrite((((1) | 64)), (0x0)))));
        break;
    default:
        if (tick % ((1000000L / 50000 /* us*/ /*us*/) / stat_LED_stat) == 0)
        {
            level_led_stat = !level_led_stat;
            ((((((1) | 64)) >= 64))?(write_595(p595, (((1) | 64)) - 64, level_led_stat)):(digitalWrite((((1) | 64)), (level_led_stat))));
        }
    }
    switch (stat_LED_task)
    {
    case 0:
        (((((((0) | 64)) >= 64))?(write_595(p595, (((0) | 64)) - 64, 0x1)):(digitalWrite((((0) | 64)), (0x1)))));
        break;
    case 255:
        (((((((0) | 64)) >= 64))?(write_595(p595, (((0) | 64)) - 64, 0x0)):(digitalWrite((((0) | 64)), (0x0)))));
        break;
    default:
        if (tick % ((1000000L / 50000 /* us*/ /*us*/) / stat_LED_task) == 0)
        {
            level_led_task = !level_led_task;
            ((((((0) | 64)) >= 64))?(write_595(p595, (((0) | 64)) - 64, level_led_task)):(digitalWrite((((0) | 64)), (level_led_task))));
        }
    }

    // set TASK LED
    if (task_slot.type == 0)
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
    time_unsaved_ms += 50000 /* us*/ /*us*/ / 1000;
    if (time_unsaved_ms >= 60000)
    {
        time_unsaved_ms = 0;
        int n_deg_diff = prot_sensor->degree - p_actprm->body.n_degree;
        if (abs(n_deg_diff) >= 10)
        {
            sync_active_params(p_actprm);
            save_active_params(p_actprm);
        }
    }

    // TASK: send web data 10 / sec
    if (tick % ((1000000L / 50000 /* us*/ /*us*/) / 5) == 1 && webSocket.count() > 0 )
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
        do { (void)0; } while (0);
        webSocket.textAll("LOG: Limit triggered.");
        if (stop_motor(pmotor))
        {
            pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
            task_slot.type = 0;
        }
    }
    // TASK: timeout limit protection each 3 sec
    if ((tick % (1000000L / 50000 /* us*/ /*us*/) == 2) &&
        (pmotor->status != 0) &&
        task_slot.type != 0 &&
        (is_calibrating == is_auto_calibrating)) // time limit tiggers either not calibrating or auto calibrating
    {
        if (abs(prot_sensor->n_deg_speed) <= 3 || (((prot_sensor->n_deg_speed) < 0) ? (-1) : (((prot_sensor->n_deg_speed) == 0) ? (0) : (1))) != pmotor->status)
        {

            // timeout limit protection triggered
            if (++(prot_sensor->stat_limit_T) >= 4)
            {
                prot_sensor->stat_limit_T = 0;
                do { (void)0; } while (0);
                webSocket.textAll("LOG: Time limit triggered.");
                while (!stop_motor(pmotor))
                    ;
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                task_slot.type = 0;
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
    if (task_slot.type == 1)
    {
        if (task_slot.n_to > 0) // time decay of manual tasks
        {
            int n_to_next = task_slot.n_to - 50000 /* us*/ /*us*/ / 1000;
            task_slot.n_to = ((n_to_next < 0) ? (0) : (n_to_next));
        }
        else if (task_slot.n_to < 0)
        {
            int n_to_next = task_slot.n_to + 50000 /* us*/ /*us*/ / 1000;
            task_slot.n_to = ((n_to_next > 0) ? (0) : (n_to_next));
        }
        else // when the life time of the manual task runs out, kill the task
        {
            task_slot.type = 0;
            while (!stop_motor(pmotor))
                ;
            pmotor->break_engage_defer = p_cfg->body.break_engage_defer; // set break engage timeout flag
            do { (void)0; } while (0);
            return;
        }
    }
    else if (task_slot.type == 2)
    {
        if ((pmotor->status == 1 && deg_now >= task_slot.n_to) || (pmotor->status == -1 && deg_now <= task_slot.n_to))
        {
            if (stop_motor(pmotor)) // return true if the motor stopping signial is confirmed
            {
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                task_slot.type = 0;
                do { (void)0; } while (0);
                return;
            }
        }
    }
    accelerate(pmotor);
    // TASK: handle break engagement waiting time (longer waiting time for heavier antenna sets)
    if (pmotor->break_engage_defer > 0)
    {
        pmotor->break_engage_defer -= (50000 /* us*/ /*us*/ / 1000);
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
        case 0: // initiate a new task
            switch (task_slot.type)
            {
            case 1:
                if (is_calibrating ||
                    (prot_sensor->is_ADC_calibrated &&
                     (((task_slot.n_to) < 0) ? (-1) : (((task_slot.n_to) == 0) ? (0) : (1))) != prot_sensor->is_limit()))
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
                    do { (void)0; } while (0);
                }
                else // kill the task if limit switches are toggled
                {
                    if (stop_motor(pmotor))
                    {
                        pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                        do { (void)0; } while (0);
                        task_slot.type = 0;
                    }
                }
                break;
            case 2:
                if (prot_sensor->is_ADC_calibrated)
                {
                    // CW CCW
                    if (task_slot.n_to > deg_now + 10 || task_slot.n_to < deg_now - 10)
                    {
                        run_CW(pmotor);
                        do { (void)0; } while (0);
                    }
                    // DEADZONE
                    else
                    {
                        task_slot.type = 0;
                        do { (void)0; } while (0);
                    }
                    task_slot.is_executed = true;
                }
                else
                {
                    task_slot.type = 0;
                    task_slot.is_executed = true;
                }
                break;
            case 0:
                task_slot.is_executed = true;
            }

            break;
        default: // alter the task
            if ((task_slot.type == 1 && (((task_slot.n_to) < 0) ? (-1) : (((task_slot.n_to) == 0) ? (0) : (1))) != pmotor->status) ||
                (task_slot.type == 2 && (((task_slot.n_to - deg_now) < 0) ? (-1) : (((task_slot.n_to - deg_now) == 0) ? (0) : (1))) != pmotor->status) ||
                task_slot.type == 0)
            {
                while (!stop_motor(pmotor))
                    ; // force stopping
                pmotor->break_engage_defer = p_cfg->body.break_engage_defer;
                task_slot.type = 0;
                do { (void)0; } while (0);
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
                           if (ArduinoOTA.getCommand() == 0)
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
                          { Serial.printf_P(
# 1081 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                           (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "MCU.ino" "." "1081" "." "96" "\", \"aSM\", @progbits, 1 #"))) = (
# 1081 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                           "Progress: %u%%\r"
# 1081 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 3
                                           ); &__pstr__[0];}))
# 1081 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
                                                                   , (progress / (total / 100))); });
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
    pinMode(4, 0x01);
    pinMode(12, 0x01);
    pinMode(13, 0x01);
    pinMode(2, 0x01);
    ((((2) >= 64))?(write_595(p595, (2) - 64, 0x1)):(digitalWrite((2), (0x1))));
    pinMode(14, 0x03);
    pinMode(0, 0x01);
    pinMode(5, 0x01);
    pinMode(16, 0x02);

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
    LittleFS.begin();
    // 485
    begin_serial485(p485, &Serial, 9600, 5, 50000 /* us*/ /*us*/ / 1000);



    //595
    begin_595(p595, 12, 13, 2, 4);
    // EEPROM
    begin_AT24C(pEEPROM, AT24C64, 0, 14, ((6) | 64), 0);
    // task slot
    init_task(&task_slot);
    // motor

    init_motor(pmotor, ((5) | 64), ((3) | 64), ((4) | 64));
    //************* read config from EEPROM ****************//
    // initialize config/param datastructures
    load_config(p_cfg);
    load_active_params(p_actprm);
    // Serial.printf("ID: %s\n", p_cfg->body.s_id);
    if (strcmp(p_cfg->body.s_id, "VORTEX2") == 0) // check if config is valid
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
            delay(50000 /* us*/ / 1000);
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
                const char* smDNSHostName = p_cfg->body.s_name;
                MDNS.begin(smDNSHostName);
                do { (void)0; } while (0);
                do { (void)0; } while (0);
                do { (void)0; } while (0);
                is_config = true;
                is_wifi_conn = true;
                // Turn on the LED
                stat_LED_stat = 1;
                (((((((1) | 64)) >= 64))?(write_595(p595, (((1) | 64)) - 64, 0x0)):(digitalWrite((((1) | 64)), (0x0)))));
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
        do { (void)0; } while (0);
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
            do { (void)0; } while (0);
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
    server.on("/getRS485", HTTP_GET, onWebGetRS485); // get RS485 recieve buffer
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
        delay(50000 /* us*/ /*us*/ / 1000);
    }
    // Serial.print(s_ip);
}


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
        ADC_smoothed = (ADC_reading - ADC_smoothed) * (2.0 / (16 + 1)) + ADC_smoothed;
    }
    if (ticker_new - ticker >= 50000 /* us*/ /*us*/)
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
