/* Copyright (c) 2017 Kameleon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_wifi_default.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi_types.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <string.h>

#include "ieee80211.h"
#define TAG "ieee80211"

//===========================================================
// LOCAL VARIABLES
static esp_netif_t *s_esp_netif = NULL;
static esp_ip4_addr_t s_ip_addr;
static QueueHandle_t s_message_queue;

//===========================================================
// FUNCTION DECLARATIONS
static bool is_our_netif(const char *prefix, esp_netif_t *netif);
static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
static void on_scan_done(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
static void on_sta_start(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

static void create_scan_event(int count, wifi_ap_record_t* input);
static void create_assoc_event(void);
static void create_connect_event(void);
static void create_disconnect_event(void);

//===========================================================
// IMPLEMENTATIONS
static void create_scan_event(int count, wifi_ap_record_t* input)
{
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    ieee80211_event_t message;
    message.code = IEEE80211_EVENT_SCAN;
    message.scan.count = count;
    message.scan.records = (ieee80211_scan_info_t*)malloc(count * sizeof(ieee80211_scan_info_t));

    // change wifi_ap_record_t to ieee80211_scan_info_t
    for ( int i = 0 ; i < count ; i++)
    {
        memcpy(message.scan.records[i].bssid, input[i].bssid, sizeof(sizeof(uint8_t)*6));
        memcpy(message.scan.records[i].ssid, input[i].ssid, sizeof(sizeof(uint8_t)*33));
        message.scan.records[i].primary_channel = input[i].primary;
        message.scan.records[i].rssi = input[i].rssi;
        message.scan.records[i].authmode = input[i].authmode;
        message.scan.records[i].cipher = input[i].pairwise_cipher;
    }

    // enqueue
    ESP_LOGD("ieee80211", "create_scan_event");
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

static void create_assoc_event(void)
{
    ESP_LOGD("ieee80211", "create_assoc_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    ieee80211_event_t message;
    message.code = IEEE80211_EVENT_ASSOC;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

static void create_connect_event(void)
{
    ESP_LOGD("ieee80211", "create_connect_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    ieee80211_event_t message;
    message.code = IEEE80211_EVENT_CONNECT;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

static void create_disconnect_event(void)
{
    ESP_LOGI("ieee80211", "create_disconnect_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    ieee80211_event_t message;
    message.code = IEEE80211_EVENT_DISCONNECT;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

//===============================================================
// PRIVATE FUNCTIONS 
static esp_netif_t *get_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;
    char *expected_desc;
    asprintf(&expected_desc, "%s: %s", TAG, desc);
    while ((netif = esp_netif_next(netif)) != NULL) {
        if (strcmp(esp_netif_get_desc(netif), expected_desc) == 0) {
            free(expected_desc);
            return netif;
        }
    }
    free(expected_desc);
    return netif;
}

/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created withing common connect component are prefixed with the module TAG,
 * so it returns true if the specified netif is owned by this module
 */
static bool is_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix)-1) == 0;
}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!is_our_netif(TAG, event->esp_netif)) {
        ESP_LOGW(TAG, "Got IPv4 from another interface \"%s\": ignored", esp_netif_get_desc(event->esp_netif));
        return;
    }
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    create_assoc_event();
}


static void on_scan_done(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    uint16_t count;
    esp_wifi_scan_get_ap_num(&count);
    wifi_ap_record_t* ap_list = malloc(count * sizeof(wifi_ap_record_t));
    esp_wifi_scan_get_ap_records(&count, ap_list);
    create_scan_event(count, ap_list);
    free(ap_list);
}

static void on_sta_start(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    create_connect_event();
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}


//===============================================================
// PUBLIC FUNCTIONS 
/**
 * Initialize the device driver.
 */
int ieee80211_init()
{
    ESP_LOGI(TAG, "ieee80211_init - wifi_start");

    s_message_queue = xQueueCreate(2, sizeof(ieee80211_event_t));

    char *desc;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Prefix the interface description with the module TAG
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
    esp_netif_config.if_desc = desc;
    esp_netif_config.route_prio = 128;
    esp_netif_t *netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    free(desc);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &on_scan_done, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &on_sta_start, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return 0;
}

/**
 * Cleanup the device driver.
 */
int ieee80211_cleanup()
{
    ESP_LOGI(TAG, "ieee80211_cleanup");

    esp_netif_t *wifi_netif = get_netif_from_desc("sta");
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &on_scan_done));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, &on_sta_start));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return -1;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);
    s_esp_netif = NULL;
    vQueueDelete(s_message_queue);
    return 0;
}

/**
 * called by io loop
 */
int ieee80211_get_event(ieee80211_event_t* out_event)
{
    BaseType_t ret = xQueueReceive(s_message_queue, out_event, 0);
    if ( ret != pdTRUE ) return -1;
    ESP_LOGD("ieee80211", "ieee80211_get_event pdTRUE");
    return 0;
}

int ieee80211_reset()
{
   // @todo Need to check how to reset the 802.11 device
    return 0;
}

/**
 * Start wifi network service and connect to AP(ssid)
 */
int ieee80211_connect(const char* _ssid, const char* _password)
{
    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid,_ssid);
    strcpy((char*)wifi_config.sta.password,_password);
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    return 0;
}

/**
 * Stop wifi network service
 */
int ieee80211_disconnect()
{
    ESP_LOGI(TAG, "ieee80211_disconnect");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    return 0;
}

/**
 * Scan AP list records.
 * @param {int} size of ap_info buffer
 * @param {ieee80211_scan_info_t*} buffer to be filled with AP records.
 */
int ieee80211_scan()
{
    ESP_LOGI(TAG, "ieee80211_scan");
    wifi_scan_config_t scan_config = {0};
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    return 0;
}

/**
 * Get AP information which is connected.
 */
int ieee80211_sta_get_ap_info(ieee80211_scan_info_t* ap_info)
{
    ESP_LOGI(TAG, "ieee80211_sta_get_ap_info");
    if ( !ap_info ) return -1;

    wifi_ap_record_t info;
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&info));
    memcpy(ap_info->bssid, info.bssid, sizeof(info.bssid));
    memcpy(ap_info->ssid, info.ssid, sizeof(info.ssid));
    ap_info->primary_channel = info.primary;
    ap_info->rssi = info.rssi;
    ap_info->authmode = info.authmode;
    ap_info->cipher = info.pairwise_cipher;
    return 0;
}



