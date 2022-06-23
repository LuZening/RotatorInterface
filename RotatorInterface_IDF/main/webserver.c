

#include "main.h"
#include "RotTask.h"
#include "webserver.h"
#include "string.h"
#include "esp_log.h"
#include "Config.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"
#define TAG "webserver"

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
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/plain");
    httpdEndHeaders(connData);
    httpdSend(connData, "reset", -1);
    /* Response END */
    // TODO: send reset semaphore
    xQueueSend(qMailboxReset, 1, 0);
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


/* Webserver URL hander: /onTask  */
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
            if(*buf == 'M')
                taskType = ROT_TASK_MANUAL;
            else if(*buf == 'T')
                taskType = ROT_TASK_TARGET;
            else if(*buf == 'N')
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
    xSemaphoreTake(mtxWebBuf1, 0);
    generate_config_string(buf1, sizeof(buf1));
    httpdStartResponse(connData, 400);
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
        char* pData = connData->post.buff;
        int nBytes = connData->post.buffLen;
        int i = 0, j = 0, k = 0; // i : start of arg name, j : position of =, k : position of &
        while(i <= nBytes)
        {
            // pick up an arg
            while(k <= nBytes)
            {
                if(pData[k] != '&' && pData[k] != '\n' && pData[k] != '\r')
                {
                    k++;
                    if(pData[k] == '=') j = k;
                }
                else
                    break;
            }
            // judge if = appears after the argument name, and & happends after =
            // note: k can be equal to j, which happens when the argument value is blank
            if(j > i && k >= j)
            {
                // tokenize the argname and arg value
                pData[j] = 0; pData[k] = 0;
                const char* argname = &(pData[i]);
                const char* argv = &(pData[j+1]);
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
                            break;
                        case CONFIG_VAR_I32:
                            val = atoi(argv);
                            isArgValueParsed = true;
                            *(int*)(pCfgItem->pV) = val;
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
                            break;
                    }
                }
                // done parsing one argument, move to the next
                i = k + 1; j = i; k = i;
            }
            // if the position of = and & does not obey the rule, parsing terminates
            else
                break;
        }
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
        xSemaphoreTake(mtxWSSending, 0);
        cgiWebsocketSend(&httpdFreertosInstance, ws, msg, lenArg, WEBSOCK_FLAG_NONE);
        xSemaphoreGive(mtxWSSending);
    }
}



// Callback function when websocket closes
static void cbWebsocketClose(Websock *ws)
{

}

// URL: /ws
static void onWebsocketConnect(Websock *ws)
{
    // register callback function when received msg
    ws->recvCb = cbWebsocketRecv;
    ws->closeCb = cbWebsocketClose;
}

int websocket_broadcast(const char* wsURL, const char* data, int len)
{
    int n = 0;
    xSemaphoreTake(mtxWSSending, 0);
    n = cgiWebsockBroadcast(&httpdFreertosInstance,  wsURL, data, len, WEBSOCK_FLAG_NONE);
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
    /* homepage END */
    ROUTE_CGI("/reset", onReset),
    ROUTE_CGI("/getSensor", onGetSensor),
    ROUTE_CGI("/netstatus", onGetNetStatus),
    ROUTE_CGI("/task", onTask), // POST only
    /* websocket */
    ROUTE_WS("/ws", onWebsocketConnect),
    ROUTE_FILESYSTEM(),

    ROUTE_END()
};



const char my_hostname[16] = "vortex";
static esp_err_t app_event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id) {
	case SYSTEM_EVENT_ETH_START:
		tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, my_hostname);
		break;
	case SYSTEM_EVENT_STA_START:
		tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, my_hostname);
		// esp_wifi_connect(); /* Calling this unconditionally would interfere with the WiFi CGI. */
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
	{
		tcpip_adapter_ip_info_t sta_ip_info;
		wifi_config_t sta_conf;
		printf("~~~~~STA~~~~~" "\n");
		if (esp_wifi_get_config(TCPIP_ADAPTER_IF_STA, &sta_conf) == ESP_OK) {
			printf("ssid: %s" "\n", sta_conf.sta.ssid);
		}

		if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &sta_ip_info) == ESP_OK) {
			printf("IP:" IPSTR "\n", IP2STR(&sta_ip_info.ip));
			printf("MASK:" IPSTR "\n", IP2STR(&sta_ip_info.netmask));
			printf("GW:" IPSTR "\n", IP2STR(&sta_ip_info.gw));
		}
		printf("~~~~~~~~~~~~~" "\n");
	}
	break;
	case SYSTEM_EVENT_STA_CONNECTED:
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		/* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
		/* Skip reconnect if disconnect was deliberate or authentication      *\
        \* failed.                                                            */
		switch(event->event_info.disconnected.reason){
		case WIFI_REASON_ASSOC_LEAVE:
		case WIFI_REASON_AUTH_FAIL:
			break;
		default:
			esp_wifi_connect();
			break;
		}
		break;
		case SYSTEM_EVENT_AP_START:
		{
			tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, my_hostname);
			tcpip_adapter_ip_info_t ap_ip_info;
			wifi_config_t ap_conf;
			printf("~~~~~AP~~~~~" "\n");
			if (esp_wifi_get_config(TCPIP_ADAPTER_IF_AP, &ap_conf) == ESP_OK) {
				printf("ssid: %s" "\n", ap_conf.ap.ssid);
				if (ap_conf.ap.authmode != WIFI_AUTH_OPEN) printf("pass: %s" "\n", ap_conf.ap.password);
			}

			if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ap_ip_info) == ESP_OK) {
				printf("IP:" IPSTR "\n", IP2STR(&ap_ip_info.ip));
				printf("MASK:" IPSTR "\n", IP2STR(&ap_ip_info.netmask));
				printf("GW:" IPSTR "\n", IP2STR(&ap_ip_info.gw));
			}
			printf("~~~~~~~~~~~~" "\n");
			// set_status_ind_wifi(WIFI_STATE_IDLE);
		}
		break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "station:" MACSTR" join,AID=%d",
					MAC2STR(event->event_info.sta_connected.mac),
					event->event_info.sta_connected.aid);

			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "station:" MACSTR"leave,AID=%d",
					MAC2STR(event->event_info.sta_disconnected.mac),
					event->event_info.sta_disconnected.aid);

			break;
		case SYSTEM_EVENT_SCAN_DONE:

			break;
		default:
			break;
	}
#ifdef USE_ETHERNET
	ethernet_handle_system_event(ctx, event);
#endif

	/* Forward event to to the WiFi CGI module */
	// cgiWifiEventCb(event);
	// update_status_ind_wifi();

	return ESP_OK;
}

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

    tcpip_adapter_init();

    httpdFreertosInit(&httpdFreertosInstance,
                    builtInUrls,
                    LISTEN_PORT,
                    connectionMemory,
                    MAX_CONNECTIONS,
                    HTTPD_FLAG_NONE);
    httpdFreertosStart(&httpdFreertosInstance);
    /* event loop BEGIN */
    ESP_ERROR_CHECK(esp_event_loop_init(app_event_handler, NULL));
    /* enable event loop END*/
}