/* Copyright (c) 2017 Kaluma
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

#include <stdlib.h>

#include "cyw43.h"
#include "err.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pico/cyw43_arch.h"
#include "pico_cyw43_magic_strings.h"
#include "system.h"

#define MAX_GPIO_NUM 2
#define SCAN_TIMEOUT 2000     /* 2 sec */
#define CONNECT_TIMEOUT 30000 /* 30 sec */

typedef struct __scan_queue_s {
  cyw43_ev_scan_result_t data;
  struct __scan_queue_s *next;
} __scan_queue_t;
typedef struct {
  uint64_t prev_time_ms;
  __scan_queue_t *p_scan_result_queue;
  bool scanning;
  uint8_t queue_size;
} __scan_result_t;
__scan_result_t *__p_scan_result;

char __current_ssid[33];
uint8_t __current_bssid[6] = {0};

JERRYXX_FUN(pico_cyw43_ctor_fn) {
  int ret = cyw43_arch_init();
  __p_scan_result = NULL;
  if (ret) {
    return jerry_create_error_from_value(create_system_error(ret), true);
  }
  cyw43_arch_enable_sta_mode();
  return jerry_create_undefined();
}

static int __check_gpio(uint32_t pin) {
  if (pin > MAX_GPIO_NUM) {
    return -1;
  }
  return 0;
}

JERRYXX_FUN(pico_cyw43_get_gpio) {
  JERRYXX_CHECK_ARG_NUMBER(0, "gpio");
  uint32_t gpio = JERRYXX_GET_ARG_NUMBER(0);
  if (__check_gpio(gpio) < 0) {
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)"GPIO pin is not exist");
  }
  bool ret = cyw43_arch_gpio_get(gpio);
  return jerry_create_boolean(ret);
}

JERRYXX_FUN(pico_cyw43_put_gpio) {
  JERRYXX_CHECK_ARG_NUMBER(0, "gpio");
  JERRYXX_CHECK_ARG_BOOLEAN(1, "value");
  uint32_t gpio = JERRYXX_GET_ARG_NUMBER(0);
  bool value = JERRYXX_GET_ARG_BOOLEAN(1);
  if (__check_gpio(gpio) < 0) {
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)"GPIO pin is not exist");
  }
  cyw43_arch_gpio_put(gpio, value);
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_ctor_fn) {
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_reset) { return jerry_create_undefined(); }

static int __scan_cb(void *env, const cyw43_ev_scan_result_t *result) {
  (void)env;
  if (__p_scan_result && __p_scan_result->scanning) {
    __p_scan_result->prev_time_ms = km_gettime();
    __scan_queue_t *p_new_node =
        (__scan_queue_t *)malloc(sizeof(__scan_queue_t));
    if (p_new_node == NULL) {
      return -1;
    }
    __p_scan_result->queue_size++;
    p_new_node->data = *result;
    p_new_node->next = NULL;
    if (__p_scan_result->p_scan_result_queue) {
      __scan_queue_t *current = __p_scan_result->p_scan_result_queue;
      while (current->next != NULL) {
        current = current->next;
      }
      current->next = p_new_node;
    } else {
      __p_scan_result->p_scan_result_queue = p_new_node;
    }
  }
  return 0;
}

JERRYXX_FUN(pico_cyw43_wifi_scan) {
  JERRYXX_CHECK_ARG_FUNCTION_OPT(0, "callback");
  if (JERRYXX_HAS_ARG(0)) {  // Do nothing if callback is NULL
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t scan_js_cb = jerry_acquire_value(callback);
    __p_scan_result = (__scan_result_t *)malloc(sizeof(__scan_result_t));
    __p_scan_result->prev_time_ms = km_gettime();
    __p_scan_result->queue_size = 0;
    __p_scan_result->p_scan_result_queue = NULL;
    __p_scan_result->scanning = true;
    cyw43_wifi_scan_options_t scan_opt = {0};
    int ret = cyw43_wifi_scan(&cyw43_state, &scan_opt, NULL, __scan_cb);
    if (ret < 0) {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      jerry_value_t errno = jerryxx_get_property_number(
          JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      jerry_value_t this_val = jerry_create_undefined();
      jerry_value_t args_p[1] = {errno};
      jerry_call_function(scan_js_cb, this_val, args_p, 1);
      jerry_release_value(errno);
      jerry_release_value(this_val);
    } else {
      uint64_t diff = 0;
      do {
        km_delay(200);
        uint64_t current_time = km_gettime();
        if (current_time < __p_scan_result->prev_time_ms) {
          diff = __UINT64_MAX__ - __p_scan_result->prev_time_ms + current_time;
        } else {
          diff = current_time - __p_scan_result->prev_time_ms;
        }
      } while (diff < SCAN_TIMEOUT);
      __p_scan_result->scanning = false;
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
      jerry_value_t scan_array =
          jerry_create_array(__p_scan_result->queue_size);
      __scan_queue_t *current = __p_scan_result->p_scan_result_queue;
      uint8_t index = 0;
      while (current) {
        jerry_value_t obj = jerry_create_object();
        char *str_buff = (char *)calloc(1, 33);
        memcpy(str_buff, current->data.ssid, current->data.ssid_len);
        jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_SSID,
                                    str_buff);
        sprintf(str_buff, "%02X:%02X:%02X:%02X:%02X:%02X",
                current->data.bssid[0], current->data.bssid[1],
                current->data.bssid[2], current->data.bssid[3],
                current->data.bssid[4], current->data.bssid[5]);
        jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_BSSID,
                                    str_buff);
        if (current->data.auth_mode == CYW43_AUTH_WPA2_MIXED_PSK) {
          sprintf(str_buff, "WPA2_WPA_PSK");
        } else if (current->data.auth_mode == CYW43_AUTH_WPA2_AES_PSK) {
          sprintf(str_buff, "WPA2_PSK");
        } else if (current->data.auth_mode == CYW43_AUTH_WPA_TKIP_PSK) {
          sprintf(str_buff, "WPA_PSK");
        } else {
          sprintf(str_buff, "OPEN");
        }
        printf("security %d\r\n", current->data.auth_mode);
        jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_SECURITY,
                                    str_buff);
        jerryxx_set_property_number(obj, MSTR_PICO_CYW43_SCANINFO_RSSI,
                                    current->data.rssi);
        jerryxx_set_property_number(obj, MSTR_PICO_CYW43_SCANINFO_CHANNEL,
                                    current->data.channel);
        jerry_value_t ret =
            jerry_set_property_by_index(scan_array, index++, obj);
        jerry_release_value(ret);
        jerry_release_value(obj);
        free(str_buff);
        __scan_queue_t *remove = current;
        current = current->next;
        free(remove);
      }
      jerry_value_t errno = jerryxx_get_property_number(
          JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
      jerry_value_t this_val = jerry_create_undefined();
      jerry_value_t args_p[2] = {errno, scan_array};
      jerry_call_function(scan_js_cb, this_val, args_p, 2);
      jerry_release_value(errno);
      jerry_release_value(this_val);
      jerry_release_value(scan_js_cb);
      jerry_release_value(scan_array);
    }
    free(__p_scan_result);
    __p_scan_result = NULL;
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_connect) {
  JERRYXX_CHECK_ARG(0, "connectInfo");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(1, "callback");
  jerry_value_t connect_info = JERRYXX_GET_ARG(0);
  jerry_value_t ssid =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_SCANINFO_SSID);
  jerry_value_t pw =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_PASSWORD);
  uint8_t *ssid_str = NULL;
  uint8_t *pw_str = NULL;
  if (jerry_value_is_string(ssid)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(ssid);
    if (len > 32) {
      len = 32;
    }
    jerryxx_string_to_ascii_char_buffer(ssid, (uint8_t *)__current_ssid, len);
    __current_ssid[len] = '\0';
  } else {
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)"SSID error");
  }
  if (jerry_value_is_string(pw)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(pw);
    pw_str = (uint8_t *)malloc(len + 1);
    jerryxx_string_to_ascii_char_buffer(pw, pw_str, len);
    pw_str[len] = '\0';
  }
  jerry_release_value(ssid);
  jerry_release_value(pw);
  // printf("ssid %s, pw %s\r\n", ssid_str, pw_str);
  int connect_ret = cyw43_arch_wifi_connect_timeout_ms(
      (char *)ssid_str, (char *)pw_str, -1, CONNECT_TIMEOUT);
  free(ssid_str);
  if (pw_str) {
    free(pw_str);
  }
  if (connect_ret) {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                0);
    jerry_value_t assoc_js_cb =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ASSOC_CB);
    jerry_value_t this_val = jerry_create_undefined();
    if (jerry_value_is_function(assoc_js_cb)) {
      jerry_call_function(assoc_js_cb, this_val, NULL, 0);
    }
    jerry_value_t connect_js_cb =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_CONNECT_CB);
    if (jerry_value_is_function(connect_js_cb)) {
      jerry_call_function(connect_js_cb, this_val, NULL, 0);
    }
    jerry_release_value(this_val);
    /** This function return RP-W mac address. need to change it */
    int mac_ret =
        cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, __current_bssid);
    if (mac_ret < 0) {
      memset(__current_bssid, 0, 6);
    }
  }
  // printf("cyw43_arch_wifi_connect_timeout_ms %d\r\n", connect_ret);
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t connect_js_cb = jerry_acquire_value(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_create_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call_function(connect_js_cb, this_val, args_p, 1);
    jerry_release_value(errno);
    jerry_release_value(this_val);
    jerry_release_value(connect_js_cb);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_disconnect) {
  JERRYXX_CHECK_ARG_FUNCTION_OPT(0, "callback");
  int disconnect_ret = cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
  if (disconnect_ret == 0) {
    jerry_value_t disconnect_js_cb = jerryxx_get_property(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_DISCONNECT_CB);
    if (jerry_value_is_function(disconnect_js_cb)) {
      jerry_value_t this_val = jerry_create_undefined();
      jerry_call_function(disconnect_js_cb, this_val, NULL, 0);
      jerry_release_value(this_val);
    }
  }
  if (JERRYXX_HAS_ARG(0)) {
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t disconnect_js_cb = jerry_acquire_value(callback);
    jerry_value_t errno;
    if (disconnect_ret) {
      errno = jerryxx_get_property_number(JERRYXX_GET_THIS,
                                          MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    } else {
      errno = jerryxx_get_property_number(JERRYXX_GET_THIS,
                                          MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
    }
    jerry_value_t this_val = jerry_create_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call_function(disconnect_js_cb, this_val, args_p, 1);
    jerry_release_value(errno);
    jerry_release_value(this_val);
    jerry_release_value(disconnect_js_cb);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_get_connection) {
  int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
  if (wifi_status == CYW43_LINK_UP) {
    jerry_value_t obj = jerry_create_object();
    jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_SSID,
                                __current_ssid);
    char *str_buff = (char *)calloc(1, 18);
    sprintf(str_buff, "%02X:%02X:%02X:%02X:%02X:%02X", __current_bssid[0],
            __current_bssid[1], __current_bssid[2], __current_bssid[3],
            __current_bssid[4], __current_bssid[5]);
    jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_BSSID, str_buff);
    free(str_buff);
    return obj;
  } else {
    return jerry_create_undefined();
  }
}

JERRYXX_FUN(pico_cyw43_network_ctor_fn) {
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                              0);
  return jerry_create_undefined();
}

JERRYXX_FUN(pico_cyw43_network_socket) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_get) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_connect) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_write) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_close) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_shutdown) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_bind) { return jerry_create_undefined(); }

JERRYXX_FUN(pico_cyw43_network_listen) { return jerry_create_undefined(); }

jerry_value_t module_pico_cyw43_init() {
  /* PICO_CYW43 class */
  jerry_value_t pico_cyw43_ctor =
      jerry_create_external_function(pico_cyw43_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property(pico_cyw43_ctor, "prototype", prototype);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_GETGPIO,
                                pico_cyw43_get_gpio);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_PUTGPIO,
                                pico_cyw43_put_gpio);
  jerry_release_value(prototype);

  jerry_value_t pico_cyw43_wifi_ctor =
      jerry_create_external_function(pico_cyw43_wifi_ctor_fn);
  jerry_value_t wifi_prototype = jerry_create_object();
  jerryxx_set_property(pico_cyw43_wifi_ctor, "prototype", wifi_prototype);
  jerryxx_set_property_function(wifi_prototype, MSTR_PICO_CYW43_WIFI_RESET,
                                pico_cyw43_wifi_reset);
  jerryxx_set_property_function(wifi_prototype, MSTR_PICO_CYW43_WIFI_SCAN,
                                pico_cyw43_wifi_scan);
  jerryxx_set_property_function(wifi_prototype, MSTR_PICO_CYW43_WIFI_CONNECT,
                                pico_cyw43_wifi_connect);
  jerryxx_set_property_function(wifi_prototype, MSTR_PICO_CYW43_WIFI_DISCONNECT,
                                pico_cyw43_wifi_disconnect);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_GET_CONNECTION,
                                pico_cyw43_wifi_get_connection);
  jerry_release_value(wifi_prototype);

  jerry_value_t pico_cyw43_network_ctor =
      jerry_create_external_function(pico_cyw43_network_ctor_fn);
  jerry_value_t network_prototype = jerry_create_object();
  jerryxx_set_property(pico_cyw43_network_ctor, "prototype", network_prototype);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_SOCKET,
                                pico_cyw43_network_socket);
  jerryxx_set_property_function(network_prototype, MSTR_PICO_CYW43_NETWORK_GET,
                                pico_cyw43_network_get);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_CONNECT,
                                pico_cyw43_network_connect);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_WRITE,
                                pico_cyw43_network_write);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_CLOSE,
                                pico_cyw43_network_close);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_SHUTDOWN,
                                pico_cyw43_network_shutdown);
  jerryxx_set_property_function(network_prototype, MSTR_PICO_CYW43_NETWORK_BIND,
                                pico_cyw43_network_bind);
  jerryxx_set_property_function(network_prototype,
                                MSTR_PICO_CYW43_NETWORK_LISTEN,
                                pico_cyw43_network_listen);
  jerry_release_value(network_prototype);

  /* pico_cyw43 module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43, pico_cyw43_ctor);
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43_WIFI,
                       pico_cyw43_wifi_ctor);
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43_NETWORK,
                       pico_cyw43_network_ctor);
  jerry_release_value(pico_cyw43_ctor);
  jerry_release_value(pico_cyw43_wifi_ctor);
  jerry_release_value(pico_cyw43_network_ctor);

  return exports;
}