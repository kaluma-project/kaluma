#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "runtime.h"
#include "global.h"
#include "jerryxx.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_event.h>

#define DEFAULT_SSID ""
#define DEFAULT_PWD ""
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#define TAG ("ieee80211dev")

JERRYXX_FUN(ieee80211dev_reset_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  printf("ieee80211dev_reset_fn\n");
  return 0;
}


static jerry_value_t ap_list_to_jobject(const wifi_ap_record_t* ap_list, int count)
{
  jerry_value_t jobj = jerry_create_array(count);
  jerry_value_t ssidStr = jerry_create_string((const jerry_char_t*)"ssid");
  jerry_value_t rssiStr = jerry_create_string((const jerry_char_t*)"rssi");
  for(int i = 0 ; i < count ; i++)
  {
    jerry_value_t elem = jerry_create_object();
    jerry_value_t ssidVal = jerry_create_string(ap_list[i].ssid);
    jerry_value_t rssiVal = jerry_create_number(ap_list[i].rssi);
    jerry_set_property(elem, ssidStr, ssidVal);
    jerry_set_property(elem, rssiStr, rssiVal);
    jerry_release_value(ssidVal);
    jerry_release_value(rssiVal);

    jerry_set_property_by_index(jobj, i, elem);
    jerry_release_value(elem);
  }
  jerry_release_value(ssidStr);
  jerry_release_value(rssiStr);

  return jobj;
}

static jerry_value_t callback_arg = 0;
static void call_callback(jerry_value_t callback)
{
    jerry_value_t nullObj = jerry_create_null();
    jerry_value_t undefined = jerry_create_undefined();
    const jerry_value_t args[2] = {nullObj, callback_arg};
    jerry_value_t ret = jerry_call_function(callback, undefined, args, 2);
    if (jerry_value_is_error(ret)) {
      jerryxx_print_error(ret, true);
    } 
    jerry_release_value(ret);
    jerry_release_value(nullObj);
    jerry_release_value(undefined);
    jerry_release_value(callback_arg);
    callback_arg = 0;
}
static void event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
      printf("WIFI_EVENT_SCAN_DONE\n");
      uint16_t count;
      esp_wifi_scan_get_ap_num(&count);
      wifi_ap_record_t* ap_list = malloc(count * sizeof(wifi_ap_record_t));
      esp_wifi_scan_get_ap_records(&count, ap_list);
      callback_arg = ap_list_to_jobject(ap_list, count);
      free(ap_list);
    } 
  /*
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: %s", ip4addr_ntoa(&event->ip_info.ip));
    }
    */
}

void wifi_start(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(jerry_value_t callback)
{
  wifi_scan_config_t scan_config = {0};
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
  ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &event_handler));
  call_callback(callback);
}

JERRYXX_FUN(ieee80211dev_scan_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  printf("ieee80211dev_scan_fn\n");
  wifi_scan(callback);
  return 0;
}

static void wifi_write(jerry_value_t chunk, jerry_value_t callback)
{
  // TODO
  printf("wifi_write\n");
}

JERRYXX_FUN(ieee80211dev_write_fn) {
  JERRYXX_CHECK_ARG_ARRAYBUFFER(0, "chunk");
  JERRYXX_CHECK_ARG_FUNCTION(1, "callback");
  jerry_value_t chunk = JERRYXX_GET_ARG(0);
  jerry_value_t callback = JERRYXX_GET_ARG(1);
  wifi_write(chunk, callback);
  return 0;
}

static void wifi_read(jerry_value_t callback)
{
  // TODO
  printf("wifi_write\n");
}

JERRYXX_FUN(ieee80211dev_read_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  wifi_read(callback);
  return 0;
}

JERRYXX_FUN(ieee80211dev_socket_fn) {
  return 0;
}

JERRYXX_FUN(ieee80211dev_get_fn) {
  return 0;
}


JERRYXX_FUN(ieee80211dev_connect_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  printf("ieee80211dev_connect_fn\n");
  return 0;
}
JERRYXX_FUN(ieee80211dev_disconnect_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  printf("ieee80211dev_disconnect_fn\n");
  return 0;
}
JERRYXX_FUN(ieee80211dev_get_connection_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  printf("ieee80211dev_get_connection_fn\n");
  return 0;
}

void register_global_ieee80211dev() {
  jerry_value_t ieee = jerry_create_object();
  jerryxx_set_property_function(ieee, "reset", ieee80211dev_reset_fn);
  jerryxx_set_property_function(ieee, "scan", ieee80211dev_scan_fn);
  jerryxx_set_property_function(ieee, "write", ieee80211dev_write_fn);
  jerryxx_set_property_function(ieee, "read", ieee80211dev_read_fn);
  jerryxx_set_property_function(ieee, "socket", ieee80211dev_socket_fn);
  jerryxx_set_property_function(ieee, "get", ieee80211dev_get_fn);
  jerryxx_set_property_function(ieee, "connect", ieee80211dev_connect_fn);
  jerryxx_set_property_function(ieee, "disconnect", ieee80211dev_disconnect_fn);
  jerryxx_set_property_function(ieee, "getConnection", ieee80211dev_get_connection_fn);
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property(global, "__ieee80211dev", ieee);
  jerry_release_value(ieee);
  jerry_release_value(global);
}

