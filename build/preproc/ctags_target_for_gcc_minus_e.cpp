# 1 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
# 1 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"



//#define __DEBUG_
// TIMER FREQUENCY = CPU FREQ / 16

# 8 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 9 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 10 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 11 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 12 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 13 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 14 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 15 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2




# 20 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 21 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 22 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 23 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2
# 24 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2

# 26 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" 2







// pin number of 485 Read/write switch
# 42 "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino"
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
    if (path.endsWith("/"))
    {
        path += "index.html";
    }

    String contentType = getContentType(path);
    String path_gz(path);
    path_gz += ".gz";
    if (SPIFFS.exists(path_gz))
    {
        AsyncWebServerResponse *response = r->beginResponse(SPIFFS, path_gz, contentType);
        response->addHeader(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "205" "." "0" "\""))) = ("Content-Encoding"); &__c[0];}))))), ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "205" "." "1" "\""))) = ("gzip"); &__c[0];}))))));
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
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "223" "." "2" "\""))) = ("name"); &__c[0];})))))))
        {
            strcpy(p_cfg->body.s_name, r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "225" "." "3" "\""))) = ("name"); &__c[0];})))))).c_str());
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "229" "." "4" "\""))) = ("ssid"); &__c[0];})))))))
        {
            strcpy(p_cfg->body.s_ssid, r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "231" "." "5" "\""))) = ("ssid"); &__c[0];})))))).c_str());
            // Serial.printf("ssid=%s\r\n", p_cfg->body.s_ssid);
            ++n_params;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "236" "." "6" "\""))) = ("passwd"); &__c[0];})))))))
        {
            strcpy(p_cfg->body.s_password, r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "238" "." "7" "\""))) = ("passwd"); &__c[0];})))))).c_str());
            ++n_params;
            strcpy(p_cfg->body.s_id, "VORTEX2");
            p_cfg->body.is_WiFi_set = true;
            is_config = true;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "244" "." "8" "\""))) = ("pot_type"); &__c[0];})))))))
        {
            p_cfg->body.pot_type = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "246" "." "9" "\""))) = ("pot_type"); &__c[0];})))))).toInt();
            prot_sensor->pot_type = (PotType)p_cfg->body.pot_type;
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "250" "." "10" "\""))) = ("multi_rounds"); &__c[0];})))))))
        {
            p_cfg->body.allow_multi_rounds = (r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "252" "." "11" "\""))) = ("multi_rounds"); &__c[0];})))))).toInt() > 0);
            prot_sensor->allow_multi_rounds = p_cfg->body.allow_multi_rounds;
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "256" "." "12" "\""))) = ("break_delay"); &__c[0];})))))))
        {
            p_cfg->body.break_engage_defer = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "258" "." "13" "\""))) = ("break_delay"); &__c[0];})))))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "261" "." "14" "\""))) = ("R_0"); &__c[0];})))))))
        {
            prot_sensor->R_0 = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "263" "." "15" "\""))) = ("R_0"); &__c[0];})))))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "266" "." "16" "\""))) = ("R_c"); &__c[0];})))))))
        {
            prot_sensor->R_c = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "268" "." "17" "\""))) = ("R_c"); &__c[0];})))))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "271" "." "18" "\""))) = ("R_max"); &__c[0];})))))))
        {
            prot_sensor->R_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "273" "." "19" "\""))) = ("R_max"); &__c[0];})))))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "276" "." "20" "\""))) = ("R_min"); &__c[0];})))))))
        {
            prot_sensor->R_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "278" "." "21" "\""))) = ("R_min"); &__c[0];})))))).toInt();
            ++n_params;
        }
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "281" "." "22" "\""))) = ("inverse_ADC"); &__c[0];})))))))
        {
            prot_sensor->inverse_ADC = (bool)(r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "283" "." "23" "\""))) = ("inverse_ADC"); &__c[0];})))))).toInt());
            ++n_params;
        }
        // Calibration Step I
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "287" "." "24" "\""))) = ("ADC_max"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "287" "." "25" "\""))) = ("ADC_min"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "287" "." "26" "\""))) = ("ADC_zero"); &__c[0];}))))))) // Step I
        {
            int ADC_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "289" "." "27" "\""))) = ("ADC_max"); &__c[0];})))))).toInt();
            int ADC_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "290" "." "28" "\""))) = ("ADC_min"); &__c[0];})))))).toInt();
            int ADC_zero = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "291" "." "29" "\""))) = ("ADC_zero"); &__c[0];})))))).toInt();
            p_cfg->body.ADC_min = ADC_min;
            p_cfg->body.ADC_max = ADC_max;
            p_cfg->body.ADC_zero = ADC_zero;
            prot_sensor->set_ADC_range(ADC_min, ADC_max, ADC_zero);
        }
        // Calibration Step 2
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "298" "." "30" "\""))) = ("CW_lim"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "298" "." "31" "\""))) = ("CCW_lim"); &__c[0];}))))))) // Step 2
        {
            int CW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "300" "." "32" "\""))) = ("CW_lim"); &__c[0];})))))).toInt();
            int CCW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "301" "." "33" "\""))) = ("CCW_lim"); &__c[0];})))))).toInt();
            p_cfg->body.deg_limit_CW = CW_lim;
            p_cfg->body.deg_limit_CCW = CCW_lim;
            prot_sensor->deg_limit_F = CW_lim;
            prot_sensor->deg_limit_B = CCW_lim;
        }
    }
    else if (r->method() == HTTP_GET)
    {
        /* code */
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "311" "." "34" "\""))) = ("manual"); &__c[0];}))))))) // manual calibration
        {
            is_calibrating = true;
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "315" "." "35" "\""))) = ("auto"); &__c[0];})))))))
        {
            if ((!is_calibrating) && pmotor->status == 0)
            {
                begin_auto_calibrate();
            }
            else
            {
                r->send(300, ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "323" "." "36" "\""))) = ("system is busy"); &__c[0];}))))));
                return;
            }
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "327" "." "37" "\""))) = ("stop"); &__c[0];})))))))
        {
            is_calibrating = false;
            is_auto_calibrating = false;
            cal_edge_count = 0;
            task_slot.type = 0;
        }
        else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "334" "." "38" "\""))) = ("CLR_rounds"); &__c[0];}))))))) // clear n_rounds = 0
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
        sprintf_P(s_httpbuffer, (__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "347" "." "39" "\""))) = ("is_calib=%d\n\n"); &__c[0];})), (int)is_calibrating);
    }
    else
    {
        sprintf_P(s_httpbuffer, (__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "351" "." "40" "\""))) = ("name=%s&ssid=%s&ip=%s&rssi=%d&pot_type=%d&multi_rounds=%d&break_delay=%d&R_0=%d&R_c=%d&R_max=%d&R_min=%d&ADC_min=%d&ADC_max=%d&ADC_zero=%d&inverse_ADC=%d&degree=%d&CW_lim=%d&CCW_lim=%d&is_calib=%d\n\n"); &__c[0];})),
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
    sprintf_P(s_httpbuffer, (__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "377" "." "41" "\""))) = ("azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"); &__c[0];})),
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
    if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "414" "." "42" "\""))) = ("type"); &__c[0];})))))) && r->hasArg("to") && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "414" "." "43" "\""))) = ("speed"); &__c[0];})))))))
    {
        float deg = prot_sensor->degree;
        int type = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "417" "." "44" "\""))) = ("type"); &__c[0];})))))).toInt();
        int to = r->arg("to").toInt();
        int to_2, to_3;
        int speed = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "420" "." "45" "\""))) = ("speed"); &__c[0];})))))).toInt();
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
            r->send(200, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "485" "." "46" "\""))) = ("task created\n"); &__c[0];}))))));
        }
        else
        {
            r->send(400, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "489" "." "47" "\""))) = ("invalid parameters\n"); &__c[0];}))))));
        }
    }
    r->send(400, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "492" "." "48" "\""))) = ("Bad request\n"); &__c[0];}))))));
}

void onHomepage(AsyncWebServerRequest *r)
{
    if (is_config)
    {
        handleFileRead(r, ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "499" "." "49" "\""))) = ("/index.html"); &__c[0];}))))));
    }
    else
    {
        handleFileRead(r, ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "503" "." "50" "\""))) = ("/config.html"); &__c[0];}))))));
    }
}

void onDebug485(AsyncWebServerRequest *r)
{
    p485->command[p485->idx_command] = 0;
    r->send(200, "text/plain", p485->command);
}

void onStatus(AsyncWebServerRequest *r)
{
    sprintf_P(s_httpbuffer,
            (__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "516" "." "51" "\""))) = ("Free Memory=%d\nWiFi mode=%d\nRSSI=%d\n\n"); &__c[0];})),
            ESP.getFreeHeap(), WiFi.getMode(), WiFi.RSSI());
    r->send(200, "text/plain", s_httpbuffer);
}

void onNotFound(AsyncWebServerRequest *r)
{
    if (!handleFileRead(r, r->url())) // if no file matches the request, return with ERROR 404
    {
        String message(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "525" "." "52" "\""))) = ("AsyncServer: File Not Found\n\n"); &__c[0];}))))));
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
        r->send(404, "text/plain", message);
    }
}

// Save config and reset
void onReset(AsyncWebServerRequest *r)
{
    r->send(200, "text/plain", ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "544" "." "53" "\""))) = ("Restarting...\n"); &__c[0];}))))));
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
        task_slot.type = 0; // stop the motor
        prot_sensor->on_edge = __null;
    }
}

void onCalibrate(AsyncWebServerRequest *r)
{
    if (r->method() == HTTP_POST)
    {
        String message(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "575" "." "54" "\""))) = ("ADC calibrate:"); &__c[0];}))))));
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "576" "." "55" "\""))) = ("ADC_max"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "576" "." "56" "\""))) = ("ADC_min"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "576" "." "57" "\""))) = ("ADC_zero"); &__c[0];}))))))) // Step I
        {
            int ADC_max = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "578" "." "58" "\""))) = ("ADC_max"); &__c[0];})))))).toInt();
            int ADC_min = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "579" "." "59" "\""))) = ("ADC_min"); &__c[0];})))))).toInt();
            int ADC_zero = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "580" "." "60" "\""))) = ("ADC_zero"); &__c[0];})))))).toInt();
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
        if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "600" "." "61" "\""))) = ("CW_lim"); &__c[0];})))))) && r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "600" "." "62" "\""))) = ("CCW_lim"); &__c[0];}))))))) // Step 2
        {
            int CW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "602" "." "63" "\""))) = ("CW_lim"); &__c[0];})))))).toInt();
            int CCW_lim = r->arg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "603" "." "64" "\""))) = ("CCW_lim"); &__c[0];})))))).toInt();
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
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "616" "." "65" "\""))) = ("manual"); &__c[0];}))))))) // manual calibration mode
    {
        is_calibrating = true;
        r->redirect("/calibrate");
    }
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "621" "." "66" "\""))) = ("auto"); &__c[0];}))))))) // GET request to initialize calibration
    {
        if ((!is_calibrating) && pmotor->status == 0)
        {
            begin_auto_calibrate();
            r->send(200, ((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "626" "." "67" "\""))) = ("Auto calibration started\n"); &__c[0];}))))));
        }
        else
        {
            r->send(300, "system is busy");
        }
    }
    else if (r->hasArg(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "633" "." "68" "\""))) = ("stop"); &__c[0];})))))))
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
    /*
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
    */

    // TASK: send web data 3 / sec
    if (tick % ((1000000L / 50000 /* us*/ /*us*/) / 3) == 1 && webSocket.count() > 0 && webSocket.availableForWriteAll())
    {
        webSocket.textAll(sensorData());
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
                while (!stop_motor(pmotor));
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
            while (!stop_motor(pmotor));
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
    ArduinoOTA.onStart([]() {
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
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf_P((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "c:\\Users\\Zening\\OneDrive\\RADIO\\Projects\\RotatorInterface\\MCU\\MCU.ino" "." "959" "." "69" "\""))) = ("Progress: %u%%\r"); &__c[0];})), (progress / (total / 100)));
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
    SPIFFS.begin();
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
        if (WiFi.softAP("VORTEX1"))
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
    server.on("/reset", onReset);
    server.on("/getSensor", onGetSensor);
    server.on("/wifiStatus", onGetWiFiStatus);
    server.on("/task", HTTP_POST, onTask);
    server.on("/debug485", onDebug485);
    server.on("/status", onStatus);
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
        ADC_smoothed = (ADC_reading - ADC_smoothed) * (2.0 / (32 + 1)) + ADC_smoothed;
    }
    if (ticker_new - ticker >= 50000 /* us*/ /*us*/)
    {
        ticker = ticker_new;
        // TASK: handle 485
        handle_serial485(p485);
        onTimer();
    }
    ArduinoOTA.handle();
}
// TODO: 485
