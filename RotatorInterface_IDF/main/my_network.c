

#include "my_network.h"

#include "esp_wifi.h"
#include "nvs.h"
#define NVS_NAMESPACE "nvs"
nvs_handle my_nvs_handle;
#define NET_CONF_KEY "netconf"
#include "esp_log.h"
#define TAG "network"
#include "sdkconfig.h"

/* *************  ETHERNET CONFIG BEGIN ************** */
esp_eth_handle_t eth_handle = NULL;

eth_mac_clock_config_t clock_config = {
    .rmii.clock_mode = EMAC_CLK_OUT,
    .rmii.clock_gpio = EMAC_CLK_OUT_180_GPIO // 17
}; /*!< EMAC Interface clock configuration */


/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}


bool ethernet_init()
{
    bool r = true;
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create new default instance of esp-netif for Ethernet
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

    // Init MAC and PHY configs to default
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    // PHY auto detect physical address
    phy_config.phy_addr = -1;
    // PHY do not use hardware reset
    phy_config.reset_gpio_num = -1;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.smi_mdc_gpio_num = GPIO_ETHERNET_MDC;
    mac_config.smi_mdio_gpio_num = GPIO_ETHERNET_MDIO;
    // MAC use GPIO17 and Output 180o reversed clock signal to EMAC 
    mac_config.clock_config = clock_config;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_EXAMPLE_ETH_PHY_LAN87XX
    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_KSZ8041
    esp_eth_phy_t *phy = esp_eth_phy_new_ksz8041(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_KSZ8081
    esp_eth_phy_t *phy = esp_eth_phy_new_ksz8081(&phy_config);
#endif
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));
    
    return r;
}


bool ethernet_start()
{
    bool r = true;
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    // CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    return r;
}

bool ethernet_stop()
{
    bool r = true;
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_stop(eth_handle));
    // CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    return r;
}

esp_err_t ethernet_handle_system_event(void *ctx, system_event_t *event){

    switch(event->event_id) {
        case SYSTEM_EVENT_ETH_GOT_IP:
        { // need bracket to declare var
            tcpip_adapter_ip_info_t ap_ip_info;
            memset(&ap_ip_info, 0, sizeof(tcpip_adapter_ip_info_t)); // clear to all zero (needed?)
            if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ap_ip_info) == 0) {
                printf("~~~~~ETH~~~~~~" "\n");
                printf("IP:" IPSTR "\n", IP2STR(&ap_ip_info.ip));
                printf("MASK:" IPSTR "\n", IP2STR(&ap_ip_info.netmask));
                printf("GW:" IPSTR "\n", IP2STR(&ap_ip_info.gw));
                printf("~~~~~~~~~~~~~~" "\n");
            }
        }
            break;

        default:
            break;
    }

    return ESP_OK;
}

/* *************  ETHERNET CONFIG END ************** */



/* *************  WIFI CONFIG BEGIN ************** */
void init_wifi(bool factory_defaults)
{
    wifi_mode_t old_mode;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    // Try to get WiFi configuration from NVS
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH)); // WIFI_STORAGE_FLASH or WIFI_STORAGE_RAM
    ESP_ERROR_CHECK(esp_wifi_get_mode(&old_mode));

    if (factory_defaults)
    {
        old_mode = WIFI_MODE_AP;
    }

    if (old_mode == WIFI_MODE_APSTA || old_mode == WIFI_MODE_STA)
    {
        //// STA settings
        wifi_config_t factory_sta_config = {
            .sta = {
                .ssid = "",
                .password = "",
            }};
        wifi_config_t sta_stored_config;
        //esp_wifi_set_mode(WIFI_MODE_APSTA); // must enable modes before trying esp_wifi_get_config()
        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &sta_stored_config));

        if (factory_defaults && strlen((char *)factory_sta_config.sta.ssid) != 0)
        {
            // load factory default STA config
            ESP_LOGI(TAG, "Using factory-default WiFi STA configuration, ssid: %s", factory_sta_config.sta.ssid);
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &factory_sta_config));
        }
        else if (strlen((char *)sta_stored_config.sta.ssid) != 0)
        {
            ESP_LOGI(TAG, "Using WiFi STA configuration from NVS, ssid: %s", sta_stored_config.sta.ssid);
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_stored_config));
        }
        else
        {
            ESP_LOGW(TAG, "No WiFi STA configuration available");
            if (old_mode == WIFI_MODE_APSTA)
                esp_wifi_set_mode(WIFI_MODE_AP); // remove STA mode
            if (old_mode == WIFI_MODE_STA)
                esp_wifi_set_mode(WIFI_MODE_NULL); // remove STA mode
        }
    }

    if (old_mode == WIFI_MODE_APSTA || old_mode == WIFI_MODE_AP)
    {
        //// AP settings
        wifi_config_t factory_ap_config;
        {
            strncpy((char *)(&factory_ap_config.ap.ssid), "VORTEX3", (sizeof((wifi_ap_config_t *)0)->ssid));
            factory_ap_config.ap.ssid_len = 0; // 0: use null termination to determine size
            factory_ap_config.ap.channel = 6;
            factory_ap_config.ap.authmode = WIFI_AUTH_OPEN; //WIFI_AUTH_WPA_WPA2_PSK; //WIFI_AUTH_OPEN;
            //strncpy((char *)(&factory_ap_config.ap.password), DEFAULT_WIFI_AP_PASS, (sizeof((wifi_ap_config_t *)0)->password));
            factory_ap_config.ap.ssid_hidden = 0;
            factory_ap_config.ap.max_connection = 4;
            factory_ap_config.ap.beacon_interval = 100;
    }
        wifi_config_t ap_stored_config;
        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_AP, &ap_stored_config));

        if (factory_defaults && strlen((char *)factory_ap_config.ap.ssid) != 0)
        {

            // load factory default STA config
            ESP_LOGI(TAG, "Using factory-default WiFi AP configuration, ssid: %s", factory_ap_config.ap.ssid);
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &factory_ap_config));
    }
        else if (strlen((char *)ap_stored_config.ap.ssid) != 0)
        {
            ESP_LOGI(TAG, "Using WiFi AP configuration from NVS, ssid: %s", ap_stored_config.ap.ssid);
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_stored_config));
    }
    else
    {
            ESP_LOGW(TAG, "No WiFi AP configuration available");
            if (old_mode == WIFI_MODE_APSTA)
                esp_wifi_set_mode(WIFI_MODE_STA); // remove AP mode
            if (old_mode == WIFI_MODE_AP)
                esp_wifi_set_mode(WIFI_MODE_NULL); // remove AP mode
    }
    }

    ESP_ERROR_CHECK( esp_wifi_start() );
}
/* *************  WIFI CONFIG END ************** */


