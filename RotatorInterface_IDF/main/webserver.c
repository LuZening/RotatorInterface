

#include "main.h"
#include "RotTask.h"
#include "webserver.h"
#include "string.h"
#include "esp_log.h"
#include "Config.h"
#include "esp_wifi.h"
// #include "tcpip_adapter.h"
#define tag "webserver"

// static const char *espfs_image_bin = WEBSERVER_FS_ADDR_BEGIN;
static char connectionMemory[sizeof(RtosConnType) * MAX_CONNECTIONS];
static HttpdFreertosInstance httpdFreertosInstance;

// http string temporary write buffer
static char buf1[1500];
SemaphoreHandle_t mtxWebBuf1;
// only 1 Websocket sending request is allowed each time
SemaphoreHandle_t mtxWSSending;

/* Webserver URL hander: /reset  */
static CgiStatus ICACHE_FLASH_ATTR onReset(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    /* Response BEGIN */
    write_to_EEPROM_async();
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/plain");
    httpdEndHeaders(connData);
    httpdSend(connData, "reset", -1);
    /* Response END */
    // TODO: send reset semaphore
    reset_async();
    return HTTPD_CGI_DONE;
}

/* Webserver URL hander: /save  */
static CgiStatus ICACHE_FLASH_ATTR onSave(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    /* Response BEGIN */
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/plain");
    httpdEndHeaders(connData);
    httpdSend(connData, "saving", -1);
    /* Response END */
    // TODO: send reset semaphore
    write_to_EEPROM_async();
    return HTTPD_CGI_DONE;
}

// char *sensorData()
// {
//     sprintf(s_httpbuffer, PSTR("azu=%d&ADC=%d&busy=%d&lmt=%d&spd=%d\n\n"),
//             prot_sensor->get_degree(),
//             prot_sensor->get_ADC(),
//             ((pmotor->status == MOT_IDLE) ? (0) : (1)),
//             prot_sensor->is_limit(),
//             prot_sensor->n_deg_speed);
//     return s_httpbuffer;
// }

/* Webserver URL hander: /onGetSensor  */
// respond with data from all sensors
static CgiStatus ICACHE_FLASH_ATTR onGetSensor(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    // TODO: 
    ESP_LOGD(tag, "onGetSensor");
    char buf[128 * N_POT_INPUTS];
    get_sensor_data_string(buf, sizeof(buf));
    /* Response BEGIN */
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/plain");
    httpdEndHeaders(connData);
    httpdSend(connData, buf, -1);
    /* Response END */
    return HTTPD_CGI_DONE;

}


/* Webserver URL hander: /onGetNetStatus  */
static CgiStatus ICACHE_FLASH_ATTR onGetNetStatus(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    char buf[256];
    // TODO
    return HTTPD_CGI_DONE;
}


/* Webserver URL hander: /task  */
// Args: 
// mot: 1, 2
// type: (M)anual or (T)arget
// to: CW(+sec) CCW(-sec) OR degree (0-360)
// speed: (0-100)
static CgiStatus ICACHE_FLASH_ATTR onTask(HttpdConnData *connData)
{
    int lenArg = 0;
    char buf[32] = {0};
    char errmsg[32] = {0};
    int nMotNo = 0;
    RotTaskType_t taskType = ROT_TASK_NULL;
    int to = 0;
    int speed100 = 0;
    RotTask_t task;
    bool isValid = true;
    ESP_LOGD(tag, "onTask");
    // TODO:
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    // only serve POST requests
    if(connData->requestType == HTTPD_METHOD_POST)
    {
        /* parse args BEGIN */
        // arg: mot
        lenArg = httpdFindArg(connData->post.buff, "mot", buf, sizeof(buf));
        if(lenArg != 0)
        {
            nMotNo = atoi(buf);
        }
        else
        {
            isValid = false;
            strncpy(errmsg, "invalid motor number", sizeof(errmsg));
            goto ON_TASK_EXIT;
        }
        // arg: type
        lenArg = httpdFindArg(connData->post.buff, "type", buf, sizeof(buf));
        if(lenArg != 0 )
        {
            if(*buf == '1')
                taskType = ROT_TASK_MANUAL;
            else if(*buf == '2')
                taskType = ROT_TASK_TARGET;
            else if(*buf == '0')
                taskType = ROT_TASK_NULL;
            else
            {
                isValid = false;
                strncpy(errmsg, "invalid task type", sizeof(errmsg));
                goto ON_TASK_EXIT;
            }
        }
        // arg: to
        lenArg = httpdFindArg(connData->post.buff, "to", buf, sizeof(buf));
        if(lenArg != 0 )
        {
            to = atoi(buf);
        }
        // arg: speed
        lenArg = httpdFindArg(connData->post.buff, "speed", buf, sizeof(buf));
        if(lenArg != 0 )
        {
            speed100 = atoi(buf);
            if(speed100 > 100)
            {
                speed100 = 100;
            }
            else if(speed100 < 0)
                speed100 = 0;
        }
        /* parse args END */
    }
ON_TASK_EXIT:
    /* Response BEGIN */
    if(isValid)
    {
        // append task to queue
        init_task(&task);
        set_task(&task, nMotNo, taskType, to, speed100);
        xQueueSendToBack(qRotTasks, &task, 0);
        httpdStartResponse(connData, 200);
        httpdHeader(connData, "Content-Type", "text/plain");
        httpdEndHeaders(connData);
        httpdSend(connData, "task received", -1);
    }
    else
    {
        httpdStartResponse(connData, 400);
        httpdHeader(connData, "Content-Type", "text/plain");
        httpdEndHeaders(connData);
        httpdSend(connData, errmsg, -1);

    }
    /* Response END */
    return HTTPD_CGI_DONE;
}


static CgiStatus ICACHE_FLASH_ATTR onGetConfig(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    // avoid multiple write of buf1, to save memory
    xSemaphoreTake(mtxWebBuf1, portMAX_DELAY);
    get_config_string(buf1, sizeof(buf1));
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/plain");
    httpdEndHeaders(connData);
    httpdSend(connData, buf1, -1);
    xSemaphoreGive(mtxWebBuf1);
    return HTTPD_CGI_DONE;
}

static CgiStatus ICACHE_FLASH_ATTR onSetConfig(HttpdConnData *connData)
{
    if (connData->isConnectionClosed) {
        return HTTPD_CGI_DONE;
    }
    if(connData->requestType == HTTPD_METHOD_POST)
    {
        const int lenbuf = 256;
        char bufR[lenbuf];
        int nParsed = 0;
        char* pData = connData->post.buff;
        int nBytes = connData->post.buffLen;
        if(nBytes >= lenbuf) nBytes = lenbuf - 1;
        strncpy(bufR, pData, nBytes);
        bufR[lenbuf] = 0;
        int i = 0, j = 0, k = 0; // i : start of arg name, j : position of =, k : position of &
        while(i <= nBytes)
        {
            // pick up an arg
            while(k <= nBytes)
            {
                if(bufR[k] != '&' && bufR[k] != '\n' && bufR[k] != '\r')
                {
                    k++;
                    if(bufR[k] == '=') j = k;
                }
                else
                    break;
            }
            // judge if = appears after the argument name, and & happends after =
            // note: k can be equal to j, which happens when the argument value is blank
            if(j > i && k >= j)
            {
                // tokenize the argname and arg value
                bufR[j] = 0; bufR[k] = 0;
                const char* argname = &(bufR[i]);
                const char* argv = &(bufR[j+1]);
                // parse
                int val;
                bool isArgValueParsed = false;
                // locate the config variable by name, if failed return NULL
                config_var_map_t* pCfgItem = get_config_variable_mapper_item_by_name(argname);
                if (pCfgItem != NULL)
                {
                    switch(pCfgItem->typ)
                    {
                        case CONFIG_VAR_U8:
                            val = atoi(argv);
                            isArgValueParsed = true;
                            *(uint8_t*)(pCfgItem->pV) = val;
                            ESP_LOGD(tag, "Set config U8 %s = %d", argname, val);
                            break;
                        case CONFIG_VAR_I32:
                            val = atoi(argv);
                            isArgValueParsed = true;
                            *(int32_t*)(pCfgItem->pV) = val;
                            ESP_LOGD(tag, "Set config I32 %s = %d", argname, val);
                            break;
                        case CONFIG_VAR_BYTESTRING:
                            val = 0;
                            /* copy string to config */
                            while(val < (CONFIG_BYTESTRING_LEN-1) && argv[val] != 0)
                            {
                                *(uint8_t*)pCfgItem->pV = argv[val];
                                val++;
                            }
                            if(val < CONFIG_BYTESTRING_LEN - 1)
                            {
                                ((uint8_t*)(pCfgItem->pV))[val + 1] = 0;
                                isArgValueParsed = true;
                            }
                            else // buffer overflowed, input string is too long
                            {
                                ((uint8_t*)(pCfgItem->pV))[val] = 0;
                                isArgValueParsed = false;
                            }
                            ESP_LOGD(tag, "Set config STR %s = %s", argname, argv);
                            break;
                    }
                    if(isArgValueParsed) 
                    {
                        ++nParsed;
                    }
                }
                // done parsing one argument, move to the next
                i = k + 1; j = i; k = i;
            }
            // if the position of = and & does not obey the rule, parsing terminates
            else
                break;
        }
        // sync values in config object with rotsensor object
        if(nParsed > 0)
        {
            push_config_to_volatile_variables(&cfg);
        }
        httpdStartResponse(connData, 200);
        httpdHeader(connData, "Content-Type", "text/plain");
        httpdEndHeaders(connData);
        sprintf(bufR, "%d", nParsed);
        httpdSend(connData, bufR, -1);
        ESP_LOGD(tag, "Set %d config items", nParsed);
    }
    return HTTPD_CGI_DONE;
}

/* Websocket handlers BEGIN */
// Callback function when websocket received msg
// Websocket msg handler
static void cbWebsocketRecv(Websock *ws, char *data, int len, int flags)
{
    char sArgValue[32] = {0};
    char msg[64] = {0};
    int lenArg = 0; 
    // command is stored in the "wscmd" argument
    lenArg = httpdFindArg(data, "wscmd", sArgValue, sizeof(sArgValue));
    if(lenArg > 0)
    {
        /* Case Test */
        // wscmd: "ping"
        if(strncmp(sArgValue, "ping", sizeof(sArgValue)) == 0)
        {

            // send ws response
            strcpy(msg, "pong");
            goto ON_WS_RECV_EXIT;
        }
        /* Case 1: received new task */
        // Args
        // wscmd: "task"
        // mot: 1, 2
        // type: (M)anual or (T)arget
        // to: CW(+sec) CCW(-sec) OR degree (0-360)
        // speed: (0-100)
        else if(strncmp(sArgValue, "task", sizeof(sArgValue)) == 0)
        {
            /* parse args BEGIN */
            int nMotNo = 1;
            int to = 0;
            int speed100 =0;
            bool isValid = true;
            RotTaskType_t taskType = ROT_TASK_NULL;
            RotTask_t task;
            // arg: mot
            lenArg = httpdFindArg(data, "mot", sArgValue, sizeof(sArgValue));
            if(lenArg != 0)
            {
                nMotNo = atoi(sArgValue);
            }
            else
            {
                isValid = false;
                strncpy(msg, "invalid motor number", sizeof(msg));
                goto ON_WS_RECV_EXIT;
            }
            // arg: type
            lenArg = httpdFindArg(data, "type", sArgValue, sizeof(sArgValue));
            if(lenArg != 0 )
            {
                if(*sArgValue == 'M')
                    taskType = ROT_TASK_MANUAL;
                else if(*sArgValue == 'T')
                    taskType = ROT_TASK_TARGET;
                else if(*sArgValue == 'N')
                    taskType = ROT_TASK_NULL;
                else
                {
                    isValid = false;
                    strncpy(msg, "invalid task type", sizeof(msg));
                    goto ON_WS_RECV_EXIT;
                }
            }
            // arg: to
            lenArg = httpdFindArg(data, "to", sArgValue, sizeof(sArgValue));
            if(lenArg != 0 )
            {
                to = atoi(sArgValue);
            }
            else {isValid = false;}
            // arg: speed
            lenArg = httpdFindArg(data, "speed", sArgValue, sizeof(sArgValue));
            if(lenArg != 0 )
            {
                speed100 = atoi(sArgValue);
                if(speed100 > 100)
                {
                    speed100 = 100;
                }
                else if(speed100 < 0)
                    speed100 = 0;
            }
            else {isValid = false;}
            /* parse args END */
            if(isValid)
            {
                // append task to queue
                init_task(&task);
                set_task(&task, nMotNo, taskType, to, speed100);
                xQueueSendToBack(qRotTasks, &task, 0);
            }
            else
            {
                // failed, send error message
            }
        }
        /* TODO: Case 2: received set config */
        else if(strncmp(sArgValue, "setconfig", sizeof(sArgValue)) == 0)
        {

        }
    }
ON_WS_RECV_EXIT:
    lenArg = strlen(msg);
    if(lenArg > 0)
    {
        xSemaphoreTake(mtxWSSending, portMAX_DELAY);
        cgiWebsocketSend(&httpdFreertosInstance.httpdInstance, ws, msg, lenArg, WEBSOCK_FLAG_NONE);
        xSemaphoreGive(mtxWSSending);
    }
}



// Callback function when websocket closes
static uint16_t nWSConns = 0;
static void cbWebsocketClose(Websock *ws)
{

    xSemaphoreTake(mtxWSSending, portMAX_DELAY);
    if(nWSConns > 0)
        nWSConns--;
    else
        ESP_LOGE(tag, "Websocket connection number encontered invalid value = %d", nWSConns - 1);
    xSemaphoreGive(mtxWSSending);
}

// URL: /ws
static void onWebsocketConnect(Websock *ws)
{
    // register callback function when received msg
    ws->recvCb = cbWebsocketRecv;
    ws->closeCb = cbWebsocketClose;
    xSemaphoreTake(mtxWSSending, portMAX_DELAY);
    nWSConns++;
    xSemaphoreGive(mtxWSSending);
}

int websocket_broadcast(const char* wsURL, const char* data, int len)
{
    int n = 0;
    xSemaphoreTake(mtxWSSending, portMAX_DELAY);
    if(nWSConns > 0)
        n = cgiWebsockBroadcast(&httpdFreertosInstance.httpdInstance,  wsURL, data, len, WEBSOCK_FLAG_NONE);
    xSemaphoreGive(mtxWSSending);
    return n;
}

/* Websocket handlers END */

/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
#define OTA_FLASH_SIZE_K 1024
#define OTA_TAGNAME "generic"

CgiUploadFlashDef uploadParams={
	.type=CGIFLASH_TYPE_FW,
	.fw1Pos=0x1000,
	.fw2Pos=((OTA_FLASH_SIZE_K*1024)/2)+0x1000,
	.fwSize=((OTA_FLASH_SIZE_K*1024)/2)-0x1000,
	.tagName=OTA_TAGNAME
};


const HttpdBuiltInUrl builtInUrls[] = {
    // ROUTE_CGI_ARG("*", cgiRedirectApClientToHostname, "esp8266.nonet"),
    //Routines to make the /wifi URL and everything beneath it work.
    //Enable the line below to protect the WiFi configuration with an username/password combo.
    //	{"/wifi/*", authBasic, myPassFn},

    // ROUTE_REDIRECT("/wifi", "/wifi/wifi.tpl"),
    // ROUTE_REDIRECT("/wifi/", "/wifi/wifi.tpl"),
    // ROUTE_CGI("/wifi/wifiscan.cgi", cgiWiFiScan),
    // ROUTE_TPL("/wifi/wifi.tpl", tplWlan),
    // ROUTE_CGI("/wifi/connect.cgi", cgiWiFiConnect),
    // ROUTE_CGI("/wifi/connstatus.cgi", cgiWiFiConnStatus),
    // ROUTE_CGI("/wifi/setmode.cgi", cgiWiFiSetMode),
    // ROUTE_CGI("/wifi/startwps.cgi", cgiWiFiStartWps),
    // ROUTE_CGI("/wifi/ap", cgiWiFiAPSettings),

    // ROUTE_WS("/websocket/ws.cgi", myWebsocketConnect),


    // Files in /static dir are assumed to never change, so send headers to encourage browser to cache forever.
    // ROUTE_FILE_EX("/static/*", &CgiOptionsEspfsStatic),
    /* homepage BEGIN */
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_REDIRECT("/index", "/index.html"),
    ROUTE_REDIRECT("/config", "/config.html"),
    /* homepage END */
    /* Functions BEGIN */
    ROUTE_CGI("/reset", onReset),
    ROUTE_CGI("/save", onSave),
    ROUTE_CGI("/getSensor", onGetSensor),
    ROUTE_CGI("/getsensor", onGetSensor),
    ROUTE_CGI("/netstatus", onGetNetStatus),
    ROUTE_CGI("/task", onTask), // POST only
    ROUTE_CGI("/getConfig", onGetConfig), 
    ROUTE_CGI("/getconfig", onGetConfig), 
    ROUTE_CGI("/setConfig", onSetConfig), // POST only 
    ROUTE_CGI("/setconfig", onSetConfig), // POST only 
    /* Functions END */

    /* websocket */
    ROUTE_WS("/ws", onWebsocketConnect),
    /* OTA BEGIN */
    ROUTE_REDIRECT("/flash", "/flash/index.html"),
	ROUTE_REDIRECT("/flash/", "/flash/index.html"),
	ROUTE_CGI("/flash/flashinfo.json", cgiGetFlashInfo),
	ROUTE_CGI("/flash/setboot", cgiSetBoot),
	ROUTE_CGI_ARG("/flash/upload", cgiUploadFirmware, &uploadParams),
	ROUTE_CGI_ARG("/flash/erase", cgiEraseFlash, &uploadParams),
	ROUTE_CGI("/flash/reboot", cgiRebootFirmware),
    /* OTA END */

    ROUTE_FILESYSTEM(),

    ROUTE_END()
};




void init_webserver()
{
    /* init OS handles */
    mtxWebBuf1 = xSemaphoreCreateMutex();
    mtxWSSending = xSemaphoreCreateMutex();
    /* init FileSystem BEGIN */
    
    // TODO: how to define espfs_image_bin
    EspFsConfig espfs_conf = {
        .memAddr = espfs_image_bin,
    };
    EspFs* fs = espFsInit(&espfs_conf);
    httpdRegisterEspfs(fs);
    /* init FileSystem END */

    esp_netif_init();

    // TCP/IP
    
    httpdFreertosInit(&httpdFreertosInstance,
                    builtInUrls,
                    LISTEN_PORT,
                    connectionMemory,
                    MAX_CONNECTIONS,
                    HTTPD_FLAG_NONE);
    httpdFreertosStart(&httpdFreertosInstance);
}