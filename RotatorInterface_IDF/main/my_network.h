#pragma once

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"

#include "libesphttpd/httpd.h"
#include "libesphttpd/httpd-espfs.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/route.h"

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "tcpip_adapter.h"

#include "libesphttpd/httpd-espfs.h"



/* *************  ETHERNET CONFIG BEGIN ************** */
extern esp_eth_handle_t eth_handle;

#define GPIO_ETHERNET_MDC 23
#define GPIO_ETHERNET_MDIO 18


extern eth_mac_clock_config_t clock_config;

bool ethernet_init();
bool ethernet_start();
bool ethernet_stop();

esp_err_t ethernet_handle_system_event(void *ctx, system_event_t *event);

/* *************  ETHERNET CONFIG END ************** */




/* *************  WIFI CONFIG BEGIN ************** */
void init_wifi(bool factory_defaults);
/* *************  WIFI CONFIG BEGIN ************** */

