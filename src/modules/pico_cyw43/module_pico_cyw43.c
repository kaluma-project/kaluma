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

#include <pico/cyw43_arch.h>
#include "err.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "system.h"

#include "dhcpserver.h"

#include "pico_cyw43_magic_strings.h"

#define MAX_GPIO_NUM 2
#define SCAN_TIMEOUT 2000     /* 2 sec */
#define CONNECT_TIMEOUT 30000 /* 30 sec */
#define MAC_STR_LENGTH 19

#define NET_SOCKET_STREAM 0 /* TCP SOCKET */
#define NET_SOCKET_DGRAM 1  /* UDP SOCKET */

#define NET_SOCKET_STATE_CLOSED 0
#define NET_SOCKET_STATE_BIND 1
#define NET_SOCKET_STATE_CONNECTED 2
#define NET_SOCKET_STATE_LISTENING 3

#define KM_MAX_SOCKET_NO 16

#define KM_CYW43_STATUS_DISABLED  0
#define KM_CYW43_STATUS_INIT      1 /* BIT 0 */
#define KM_CYW43_STATUS_DNS_DONE  2 /* BIT 1 */
#define KM_CYW43_STATUS_AP_MODE   4 /* BIT 2 */

#define CYW43_WIFI_AUTH_OPEN      0
#define CYW43_WIFI_AUTH_WEP_PSK   1 /* BIT 0 */
#define CYW43_WIFI_AUTH_WPA       2 /* BIT 1 */
#define CYW43_WIFI_AUTH_WPA2      4 /* BIT 2 */

typedef struct {
  int8_t fd;
  int8_t server_fd;
  int8_t ptcl;
  int8_t state;
  uint16_t lport;
  uint16_t rport;
  jerry_value_t obj;
  ip_addr_t raddr;
  union {
    struct tcp_pcb *tcp_pcb;
    struct udp_pcb *udp_pcb;
  };
  struct tcp_pcb *tcp_server_pcb;
} __socket_data_t;

typedef struct {
  __socket_data_t socket[KM_MAX_SOCKET_NO];
  ip_addr_t laddr;
} __socket_t;

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

typedef struct {
  volatile uint8_t status_flag;
  char current_ssid[33];
  char current_bssid[18];
  char ap_gateway_ip4[16];
} __cyw43_t;

__cyw43_t __cyw43_drv;
__scan_result_t *__p_scan_result;
__socket_t __socket_info;

dhcp_server_t dhcp_server;

bool km_is_valid_fd(int8_t fd) {
  if ((fd >= 0) && (fd < KM_MAX_SOCKET_NO)) {
    return true;
  }
  return false;
}

int8_t km_get_socket_fd(void) {
  for (int i = 0; i < KM_MAX_SOCKET_NO; i++) {
    if (__socket_info.socket[i].fd != i) {
      __socket_info.socket[i].fd = i;
      return i;
    }
  }
  return EMFILE;  // Too many sockets.
}

static err_t __tcp_close(struct tcp_pcb *pcb) {
  cyw43_arch_lwip_check();
  err_t err = tcp_close(pcb);
  if (err != ERR_OK) {
    tcp_abort(pcb);
    err = ERR_ABRT;
  }
  return err;
}

void km_cyw43_deinit() {
  cyw43_arch_lwip_begin();
  for (int i = 0; i < KM_MAX_SOCKET_NO; i++) {
    if (__socket_info.socket[i].fd >= 0) {
      if (__socket_info.socket[i].obj != 0)
        jerry_value_free(__socket_info.socket[i].obj);
      __socket_info.socket[i].obj = 0;
      if (__socket_info.socket[i].ptcl == NET_SOCKET_STREAM) {
        if (__socket_info.socket[i].tcp_pcb) {
          __tcp_close(__socket_info.socket[i].tcp_pcb);
        }
        if (__socket_info.socket[i].tcp_server_pcb) {
          __tcp_close(__socket_info.socket[i].tcp_server_pcb);
        }
      } else {
        if (__socket_info.socket[i].udp_pcb) {
          udp_remove(__socket_info.socket[i].udp_pcb);
        }
      }
      __socket_info.socket[i].fd = -1;
    }
  }
  __cyw43_drv.status_flag = KM_CYW43_STATUS_DISABLED;
  cyw43_arch_lwip_end();
  cyw43_arch_deinit();
  cyw43_hal_pin_low(CYW43_PIN_WL_REG_ON);
  km_delay(50);
  cyw43_hal_pin_high(CYW43_PIN_WL_REG_ON);
  km_delay(200);
}

void km_cyw43_infinite_loop() {
  if (__cyw43_drv.status_flag & KM_CYW43_STATUS_INIT) {
#ifndef NDEBUG
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, (km_gettime() / 500) % 2 == 0 ? 1 : 0);
#endif
    cyw43_arch_poll();
  }
}

static int __cyw43_init() {
  int ret = 0;
  if (__cyw43_drv.status_flag == KM_CYW43_STATUS_DISABLED) {
    ret = cyw43_arch_init();
    if (ret == 0) {
      __p_scan_result = NULL;
      cyw43_arch_enable_sta_mode();
      __cyw43_drv.status_flag |= KM_CYW43_STATUS_INIT;
    }
  }
  return ret;
}

JERRYXX_FUN(pico_cyw43_ctor_fn) {
  if (__cyw43_init()) {
    return jerry_exception_value(create_system_error(EAGAIN), true);
  }
  return jerry_undefined();
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
    return jerry_error_sz(JERRY_ERROR_TYPE,
                              "GPIO pin is not exist");
  }
  bool ret = cyw43_arch_gpio_get(gpio);
  return jerry_boolean(ret);
}

JERRYXX_FUN(pico_cyw43_put_gpio) {
  JERRYXX_CHECK_ARG_NUMBER(0, "gpio");
  JERRYXX_CHECK_ARG_BOOLEAN(1, "value");
  uint32_t gpio = JERRYXX_GET_ARG_NUMBER(0);
  bool value = JERRYXX_GET_ARG_BOOLEAN(1);
  if (__check_gpio(gpio) < 0) {
    return jerry_error_sz(JERRY_ERROR_TYPE,
                              "GPIO pin is not exist");
  }
  cyw43_arch_gpio_put(gpio, value);
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_ctor_fn) {
  if (__cyw43_init()) {
    return jerry_exception_value(create_system_error(EAGAIN), true);
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_reset) {
  JERRYXX_CHECK_ARG_FUNCTION_OPT(0, "callback");
  __cyw43_drv.status_flag = KM_CYW43_STATUS_DISABLED;
  km_cyw43_deinit();
  km_delay(500);
  if (__cyw43_init()) {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                -1);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                0);
  }
  if (JERRYXX_HAS_ARG(0)) {
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t reset_js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(reset_js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(reset_js_cb);
  }
  return jerry_undefined();
}

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
    jerry_value_t scan_js_cb = jerry_value_copy(callback);
    __p_scan_result = (__scan_result_t *)malloc(sizeof(__scan_result_t));
    __p_scan_result->prev_time_ms = km_gettime();
    __p_scan_result->queue_size = 0;
    __p_scan_result->p_scan_result_queue = NULL;
    __p_scan_result->scanning = true;
    cyw43_wifi_scan_options_t scan_opt = {0};
    int ret = cyw43_wifi_scan(&cyw43_state, &scan_opt, NULL, __scan_cb);
    if (ret < 0) {
      jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                  -1);
      jerry_value_t errno = jerryxx_get_property_number(
          JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, -1);
      jerry_value_t this_val = jerry_undefined();
      jerry_value_t args_p[1] = {errno};
      jerry_call(scan_js_cb, this_val, args_p, 1);
      jerry_value_free(errno);
      jerry_value_free(this_val);
    } else {
      while(cyw43_wifi_scan_active(&cyw43_state)) {
        __p_scan_result->scanning = true;
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(20));
#else
        km_delay(20);
#endif
      }
      __p_scan_result->scanning = false;
      jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                  0);
      jerry_value_t scan_array =
          jerry_array(__p_scan_result->queue_size);
      __scan_queue_t *current = __p_scan_result->p_scan_result_queue;
      uint8_t index = 0;
      while (current) {
        jerry_value_t obj = jerry_object();
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
        if ((current->data.auth_mode & (CYW43_WIFI_AUTH_WPA | CYW43_WIFI_AUTH_WPA2)) == (CYW43_WIFI_AUTH_WPA | CYW43_WIFI_AUTH_WPA2)) {
          sprintf(str_buff, "WPA2_WPA_PSK");
        } else if (current->data.auth_mode & CYW43_WIFI_AUTH_WPA2) {
          sprintf(str_buff, "WPA2_PSK");
        } else if (current->data.auth_mode & CYW43_WIFI_AUTH_WPA) {
          sprintf(str_buff, "WPA_PSK");
        } else if (current->data.auth_mode & CYW43_WIFI_AUTH_WEP_PSK) {
          sprintf(str_buff, "WEP_PSK");
        } else if (current->data.auth_mode == CYW43_WIFI_AUTH_OPEN) {
          sprintf(str_buff, "OPEN");
        } else {
          sprintf(str_buff, "-"); // Unknown
        }
        jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_SECURITY,
                                    str_buff);
        jerryxx_set_property_number(obj, MSTR_PICO_CYW43_SCANINFO_RSSI,
                                    current->data.rssi);
        jerryxx_set_property_number(obj, MSTR_PICO_CYW43_SCANINFO_CHANNEL,
                                    current->data.channel);
        jerry_value_t ret =
            jerry_object_set_index(scan_array, index++, obj);
        jerry_value_free(ret);
        jerry_value_free(obj);
        free(str_buff);
        __scan_queue_t *remove = current;
        current = current->next;
        free(remove);
      }
      jerry_value_t errno = jerryxx_get_property_number(
          JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
      jerry_value_t this_val = jerry_undefined();
      jerry_value_t args_p[2] = {errno, scan_array};
      jerry_call(scan_js_cb, this_val, args_p, 2);
      jerry_value_free(errno);
      jerry_value_free(this_val);
      jerry_value_free(scan_js_cb);
      jerry_value_free(scan_array);
    }
    free(__p_scan_result);
    __p_scan_result = NULL;
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_connect) {
  JERRYXX_CHECK_ARG(0, "connectInfo");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(1, "callback");
  jerry_value_t connect_info = JERRYXX_GET_ARG(0);
  jerry_value_t ssid =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_SCANINFO_SSID);
  jerry_value_t bssid =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_SCANINFO_BSSID);
  uint8_t *bssid_ptr = NULL;
  jerry_value_t pw =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_PASSWORD);
  uint8_t *pw_str = NULL;
  jerry_value_t security =
      jerryxx_get_property(connect_info, MSTR_PICO_CYW43_SCANINFO_SECURITY);
  uint32_t auth = CYW43_AUTH_OPEN; // Default is OPEN
  if (jerry_value_is_string(ssid)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(ssid);
    if (len > 32) {
      len = 32;
    }
    jerryxx_string_to_ascii_char_buffer(
        ssid, (uint8_t *)__cyw43_drv.current_ssid, len);
    __cyw43_drv.current_ssid[len] = '\0';
  } else {
    return jerry_error_sz(JERRY_ERROR_TYPE,
                              "SSID error");
  }
  if (jerry_value_is_string(bssid)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(bssid);
    jerryxx_string_to_ascii_char_buffer(
        bssid, (uint8_t *)__cyw43_drv.current_bssid, len);
    __cyw43_drv.current_bssid[len] = '\0';

    uint8_t bssid_arr[6];
    if (6 == sscanf(__cyw43_drv.current_bssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
        &bssid_arr[5], &bssid_arr[4], &bssid_arr[3],
        &bssid_arr[2], &bssid_arr[1], &bssid_arr[0])){
      bssid_ptr = bssid_arr;
    }
  }
  if (jerry_value_is_string(pw)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(pw);
    pw_str = (uint8_t *)malloc(len + 1);
    jerryxx_string_to_ascii_char_buffer(pw, pw_str, len);
    if (len >= 8) { // Min lenght of the WPA is 8.
      auth = CYW43_AUTH_WPA2_MIXED_PSK; // Default auth is changed.
    }
    pw_str[len] = '\0';
  }
  if (jerry_value_is_string(security)) {
    jerry_size_t len = jerryxx_get_ascii_string_size(security);
    uint8_t *security_str = (uint8_t *)malloc(len + 1);
    jerryxx_string_to_ascii_char_buffer(security, security_str, len);
    security_str[len] = '\0';
    if (!strcmp((const char *)security_str, "WPA2_WPA_PSK")) {
      auth = CYW43_AUTH_WPA2_MIXED_PSK;
    } else if (!strcmp((const char *)security_str, "WPA2_PSK")) {
      auth = CYW43_AUTH_WPA2_AES_PSK;
    } else if (!strcmp((const char *)security_str, "WPA_PSK")) {
      auth = CYW43_WIFI_AUTH_WPA;
    } else if (!strcmp((const char *)security_str, "WEP_PSK")) {
      auth = 0x00100001; // no idea if this works
    } else if (!strcmp((const char *)security_str, "OPEN")) {
      auth = CYW43_AUTH_OPEN;
    }
    free(security_str);
  }
  jerry_value_free(ssid);
  jerry_value_free(bssid);
  jerry_value_free(pw);
  jerry_value_free(security);
  int connect_ret = cyw43_arch_wifi_connect_bssid_timeout_ms(
      (char *)__cyw43_drv.current_ssid, bssid_ptr, (char *)pw_str, auth, CONNECT_TIMEOUT);
  if (pw_str) {
    free(pw_str);
  }
  if (connect_ret) {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                -1);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                0);
    jerry_value_t connect_js_cb =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_CONNECT_CB);
    jerry_value_t assoc_js_cb =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ASSOC_CB);
    jerry_value_t this_val = jerry_undefined();
    if (jerry_value_is_function(assoc_js_cb)) {
      jerry_call(assoc_js_cb, this_val, NULL, 0);
    }
    if (jerry_value_is_function(connect_js_cb)) {
      jerry_call(connect_js_cb, this_val, NULL, 0);
    }
    jerry_value_free(connect_js_cb);
    jerry_value_free(this_val);
    jerry_value_free(assoc_js_cb);

    /** I can't find the way to get connected device mac address on pico-w SDK.
    // This function return RP-W mac address. need to change it
    int mac_ret =
        cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, __current_bssid);
    if (mac_ret < 0) {
      memset(__current_bssid, 0, 6);
    } */
  }
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t connect_js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(connect_js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(connect_js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_disconnect) {
  JERRYXX_CHECK_ARG_FUNCTION_OPT(0, "callback");
  int disconnect_ret = cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
  if (disconnect_ret == 0) {
    jerry_value_t disconnect_js_cb = jerryxx_get_property(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_DISCONNECT_CB);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                0);
    if (jerry_value_is_function(disconnect_js_cb)) {
      jerry_value_t this_val = jerry_undefined();
      jerry_call(disconnect_js_cb, this_val, NULL, 0);
      jerry_value_free(this_val);
    }
    jerry_value_free(disconnect_js_cb);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                -1);
  }
  if (JERRYXX_HAS_ARG(0)) {
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_get_connection) {
  JERRYXX_CHECK_ARG_FUNCTION_OPT(0, "callback");
  int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
  jerry_value_t obj = jerry_object();
  if (wifi_status == CYW43_LINK_UP) {
    jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_SSID,
                                __cyw43_drv.current_ssid);
    /** Can't find the way to get connected device mac address in
    the pico-w SDK.
    char *str_buff = (char *)calloc(1, 18); sprintf(str_buff,
    "%02X:%02X:%02X:%02X:%02X:%02X", __current_bssid[0],
            __current_bssid[1], __current_bssid[2], __current_bssid[3],
            __current_bssid[4], __current_bssid[5]);
    */
    jerryxx_set_property_string(obj, MSTR_PICO_CYW43_SCANINFO_BSSID,
                                __cyw43_drv.current_bssid);
    /* free(str_buff); */
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                0);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                                -1);
  }
  if (JERRYXX_HAS_ARG(0)) {
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t get_connect_js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    if (wifi_status == CYW43_LINK_UP) {
      jerry_value_t args_p[2] = {errno, obj};
      jerry_call(get_connect_js_cb, this_val, args_p, 2);
    } else {
      jerry_value_t args_p[1] = {errno};
      jerry_call(get_connect_js_cb, this_val, args_p, 1);
    }
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(get_connect_js_cb);
  }
  jerry_value_free(obj);
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_ctor_fn) {
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                              0);
  for (int i = 0; i < KM_MAX_SOCKET_NO; i++) {
    __socket_info.socket[i].fd = -1;
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_socket) {
  JERRYXX_CHECK_ARG_STRING(0, "domain");
  JERRYXX_CHECK_ARG_STRING(1, "protocol");
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, domain);
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, protocol);
  int protocol_param = NET_SOCKET_STREAM;
  if (strcmp(protocol, "STREAM") && strcmp(protocol, "DGRAM")) {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "un-supported domain or protocol.");
  } else {
    if (strcmp(protocol, "STREAM")) {
      protocol_param = NET_SOCKET_DGRAM;
    }
  }
  if (__cyw43_drv.status_flag == KM_CYW43_STATUS_DISABLED) {
    return jerry_error_sz(
        JERRY_ERROR_COMMON,
        "PICO-W CYW43 WiFi is not initialized.");
  }
  int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
  if ((wifi_status != CYW43_LINK_UP) && ((__cyw43_drv.status_flag & KM_CYW43_STATUS_AP_MODE) == 0)) {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "WiFi is not connected.");
  }
  int8_t fd = km_get_socket_fd();
  if (!km_is_valid_fd(fd)) {
    return jerry_exception_value(create_system_error(EREMOTEIO), true);
  }
  __socket_info.socket[fd].server_fd = -1;
  __socket_info.socket[fd].tcp_server_pcb = NULL;
  __socket_info.socket[fd].state = NET_SOCKET_STATE_CLOSED;
  __socket_info.socket[fd].ptcl = protocol_param;
  __socket_info.socket[fd].lport = 0;
  __socket_info.socket[fd].rport = 0;
  __socket_info.socket[fd].raddr.addr = 0;

  // The socket should be allocated as long as this object we are about to create exists..
  __socket_info.socket[fd].obj = jerry_object();

  uint8_t mac_addr[6] = {0};
  char p_str_buff[18];
  if (cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac_addr) < 0) {
    memset(mac_addr, 0, 6);
  }
  sprintf(p_str_buff, "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1],
          mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_MAC,
                              p_str_buff);
  jerryxx_set_property_number(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_FD, fd);
  if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
    sprintf(p_str_buff, "STREAM");
  } else {
    sprintf(p_str_buff, "DGRAM");
  }
  jerryxx_set_property_string(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_PTCL, p_str_buff);
  jerryxx_set_property_number(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_STATE,
                              __socket_info.socket[fd].state);
  if (__cyw43_drv.status_flag & KM_CYW43_STATUS_AP_MODE) {
    sprintf(p_str_buff, "%s", __cyw43_drv.ap_gateway_ip4);
  } else {
    struct netif *p_netif = &(cyw43_state.netif[CYW43_ITF_STA]);
    const ip_addr_t *laddr = netif_ip_addr4(p_netif);
    __socket_info.laddr = *laddr;
    strncpy(p_str_buff, ipaddr_ntoa(laddr), sizeof(p_str_buff) - 1);
  }
  jerryxx_set_property_string(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_LADDR, p_str_buff);
  jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_IP,
                              p_str_buff);
  jerryxx_set_property_number(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_LPORT,
                              __socket_info.socket[fd].lport);
  strncpy(p_str_buff, ipaddr_ntoa(&(__socket_info.socket[fd].raddr)), sizeof(p_str_buff) - 1);
  jerryxx_set_property_string(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_RADDR, p_str_buff);
  jerryxx_set_property_number(__socket_info.socket[fd].obj,
                              MSTR_PICO_CYW43_SOCKET_RPORT,
                              __socket_info.socket[fd].rport);

  return jerry_number(fd);
}

JERRYXX_FUN(pico_cyw43_network_get) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  if (!km_is_valid_fd(fd) || !km_is_valid_fd(__socket_info.socket[fd].fd)) {
    return jerry_undefined();
  }
  return jerry_value_copy(__socket_info.socket[fd].obj);
}

static err_t __net_socket_close(int8_t fd) {
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd)) {
    __socket_info.socket[fd].state = NET_SOCKET_STATE_CLOSED;
    __socket_info.socket[fd].fd = -1;
    __socket_info.socket[fd].server_fd = -1;
  } else {
    return EPERM;
  }
  cyw43_arch_lwip_begin();
  if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
    if (__socket_info.socket[fd].tcp_server_pcb != NULL) {
      tcp_arg(__socket_info.socket[fd].tcp_server_pcb, NULL);
      err = __tcp_close(__socket_info.socket[fd].tcp_server_pcb);
      __socket_info.socket[fd].tcp_server_pcb = NULL;
    }
    if (__socket_info.socket[fd].tcp_pcb != NULL) {
      __tcp_close(__socket_info.socket[fd].tcp_pcb);
      __socket_info.socket[fd].tcp_pcb = NULL;
    }
  } else { /** UDP */
    if (__socket_info.socket[fd].udp_pcb != NULL) {
      udp_disconnect(__socket_info.socket[fd].udp_pcb);
      udp_remove(__socket_info.socket[fd].udp_pcb);
      __socket_info.socket[fd].udp_pcb = NULL;
    }
  }
  cyw43_arch_lwip_end();
  if (__socket_info.socket[fd].obj == 0) {
    return err;
  }
  jerry_value_t close_js_cb = jerryxx_get_property(
      __socket_info.socket[fd].obj, MSTR_PICO_CYW43_SOCKET_CLOSE_CB);
  if (jerry_value_is_function(close_js_cb)) {
    jerry_value_t this_val = jerry_undefined();
    jerry_call(close_js_cb, this_val, NULL, 0);
    jerry_value_free(this_val);
  }
  jerry_value_free(close_js_cb);
  if (__socket_info.socket[fd].obj != 0)
    jerry_value_free(__socket_info.socket[fd].obj);
  __socket_info.socket[fd].obj = 0;
  return err;
}

static err_t __net_data_received(int8_t fd, struct tcp_pcb *tpcb,
                                struct pbuf *p) {
  err_t err = ERR_OK;
  if (p == NULL) {
    if (__socket_info.socket[fd].state != NET_SOCKET_STATE_CLOSED || __socket_info.socket[fd].obj != 0)
      err = __net_socket_close(fd);
  } else {
    int8_t read_fd = km_is_valid_fd(__socket_info.socket[fd].server_fd)
                          ? __socket_info.socket[fd].server_fd : fd;
    if (km_is_valid_fd(read_fd)) {
      if (__socket_info.socket[read_fd].state == NET_SOCKET_STATE_CLOSED)
        return err;
      if (p->tot_len > 0) {
        uint8_t *receiver_buffer = (uint8_t *)malloc(sizeof(uint8_t) * p->tot_len);
        uint32_t buff_offset = 0;
        for (struct pbuf *q = p; q != NULL; q = q->next) {
          memcpy((uint8_t *)(receiver_buffer + buff_offset), q->payload, q->len);
          buff_offset += q->len;
        }
        if (tpcb) {
          cyw43_arch_lwip_check();
          tcp_recved(tpcb, p->tot_len);
        }
        if ( __socket_info.socket[read_fd].obj == 0) {
          free(receiver_buffer);
          return err;
        }
        jerry_value_t read_js_cb = jerryxx_get_property(
            __socket_info.socket[read_fd].obj, MSTR_PICO_CYW43_SOCKET_READ_CB);
        if (jerry_value_is_function(read_js_cb)) {
          jerry_value_t this_val = jerry_undefined();
          jerry_value_t buffer =
              jerry_arraybuffer_external(receiver_buffer, p->tot_len, NULL);
          jerry_value_t data = jerry_typedarray_with_buffer(
              JERRY_TYPEDARRAY_UINT8, buffer);
          jerry_value_free(buffer);
          jerry_value_t args_p[1] = {data};
          jerry_call(read_js_cb, this_val, args_p, 2);
          jerry_value_free(data);
          jerry_value_free(this_val);
        } else {
          free(receiver_buffer);
        }
        jerry_value_free(read_js_cb);
      }
    } else {
      err = ERANGE;
    }
    pbuf_free(p);
  }
  return err;
}

static err_t __tcp_data_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                                err_t err) {
  if (err == ERR_OK) {
    int8_t *fd = (int8_t *)arg;
    err = __net_data_received(*fd, tpcb, p);
  }
  return err;
}

static void __udp_data_recv_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                               const struct ip4_addr *addr,
                               short unsigned int port) {
  (void)upcb;
  (void)addr;
  (void)port;
  int8_t *fd = (int8_t *)arg;
  __net_data_received(*fd, NULL, p);
}

static err_t __net_client_connect_cb(void *arg, struct tcp_pcb *tpcb,
                                     err_t err) {
  if (err == ERR_OK) {
    int8_t *fd = (int8_t *)arg;
    if (km_is_valid_fd(*fd)) {
      if (__socket_info.socket[*fd].state == NET_SOCKET_STATE_CLOSED)
        return err;
      jerry_value_t connect_js_cb = jerryxx_get_property(
          __socket_info.socket[*fd].obj, MSTR_PICO_CYW43_SOCKET_CONNECT_CB);
      if (jerry_value_is_function(connect_js_cb)) {
        jerry_value_t this_val = jerry_undefined();
        jerry_call(connect_js_cb, this_val, NULL, 0);
        jerry_value_free(this_val);
      }
      jerry_value_free(connect_js_cb);
    } else {
      err = ERANGE;
    }
  }
  return err;
}

static err_t __tcp_server_accept_cb(void *arg, struct tcp_pcb *newpcb,
                                    err_t err) {
  if (err == ERR_OK) {
    int8_t *server_fd = (int8_t *)arg;
    int8_t fd = km_get_socket_fd();
    if (km_is_valid_fd(*server_fd) && km_is_valid_fd(fd)) {
      char *p_str_buff = (char *)malloc(18);
      __socket_info.socket[fd].server_fd = *server_fd;
      __socket_info.socket[fd].tcp_server_pcb = NULL;
      __socket_info.socket[fd].state = NET_SOCKET_STATE_CONNECTED;
      __socket_info.socket[fd].ptcl = NET_SOCKET_STREAM;
      __socket_info.socket[fd].lport = __socket_info.socket[*server_fd].lport;
      __socket_info.socket[fd].rport = 0;
      __socket_info.socket[fd].raddr.addr = 0;
      __socket_info.socket[fd].obj = jerry_object();
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_FD, fd);
      sprintf(p_str_buff, "STREAM");
      jerryxx_set_property_string(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_PTCL, p_str_buff);
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_STATE,
                                  __socket_info.socket[fd].state);
      struct netif *p_netif = &(cyw43_state.netif[CYW43_ITF_STA]);
      const ip_addr_t *laddr = netif_ip_addr4(p_netif);
      __socket_info.laddr = *laddr;
      sprintf(p_str_buff, "%s", ipaddr_ntoa(laddr));
      jerryxx_set_property_string(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_LADDR, p_str_buff);
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_LPORT,
                                  __socket_info.socket[fd].lport);
      sprintf(p_str_buff, "%s", ipaddr_ntoa(&(__socket_info.socket[fd].raddr)));
      jerryxx_set_property_string(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_RADDR, p_str_buff);
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_RPORT,
                                  __socket_info.socket[fd].rport);
      free(p_str_buff);
      __socket_info.socket[fd].tcp_pcb = newpcb;
      cyw43_arch_lwip_check();
      tcp_arg(__socket_info.socket[fd].tcp_pcb, &(__socket_info.socket[fd].fd));
      tcp_poll(__socket_info.socket[fd].tcp_pcb, NULL, 0);
      tcp_sent(__socket_info.socket[fd].tcp_pcb, NULL);
      tcp_err(__socket_info.socket[fd].tcp_pcb, NULL);
      tcp_recv(__socket_info.socket[fd].tcp_pcb, __tcp_data_recv_cb);
      jerry_value_t acept_js_cb =
          jerryxx_get_property(__socket_info.socket[*server_fd].obj,
                               MSTR_PICO_CYW43_SOCKET_ACCEPT_CB);
      if (jerry_value_is_function(acept_js_cb)) {
        jerry_value_t this_val = jerry_undefined();
        jerry_value_t fd_val = jerry_number(fd);
        jerry_value_t args_p[1] = {fd_val};
        jerry_call(acept_js_cb, this_val, args_p, 1);
        jerry_value_free(fd_val);
        jerry_value_free(this_val);
      }
      jerry_value_free(acept_js_cb);
    } else {
      err = ECONNREFUSED;
    }
  }
  return err;
}

void __dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
  (void) name;
  cyw43_arch_lwip_check();
  if (ipaddr) {
    *(ip_addr_t *)callback_arg = *ipaddr;
  } else {
    IP4_ADDR((ip_addr_t *)callback_arg, 0, 0, 0, 0); // IP is not found.
  }
  __cyw43_drv.status_flag |= KM_CYW43_STATUS_DNS_DONE;
}

JERRYXX_FUN(pico_cyw43_network_connect) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_STRING(1, "addr");
  JERRYXX_CHECK_ARG_NUMBER(2, "port");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(3, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, addr_str);
  uint16_t port = JERRYXX_GET_ARG_NUMBER(2);
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd) && __socket_info.socket[fd].state == NET_SOCKET_STATE_CLOSED) {
    cyw43_arch_lwip_begin();
    __cyw43_drv.status_flag &= ~KM_CYW43_STATUS_DNS_DONE;
    err = dns_gethostbyname_addrtype((const char *)addr_str, &(__socket_info.socket[fd].raddr),
                                      __dns_found_cb, &(__socket_info.socket[fd].raddr),
                                      LWIP_DNS_ADDRTYPE_IPV4);
    cyw43_arch_lwip_end();
    if (err == ERR_INPROGRESS) {
      int16_t timeout = 300; // 3 Sec
      while((__cyw43_drv.status_flag & KM_CYW43_STATUS_DNS_DONE) == 0) {
        if (timeout-- <= 0) {
          jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
          return jerry_error_sz(JERRY_ERROR_COMMON,
                                  "DNS response timeout.");
          }
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10));
#else
        km_delay(10);
#endif
      }
      if (ip4_addr_get_u32(&(__socket_info.socket[fd].raddr)) == 0) {
        jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
        return jerry_error_sz(JERRY_ERROR_COMMON,
                                 "DNS Error: IP is not found.");
      }
      __cyw43_drv.status_flag &= ~KM_CYW43_STATUS_DNS_DONE;
    } else if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      return jerry_error_sz(JERRY_ERROR_COMMON,
                                "DNS Error: DNS access error.");
    }
    __socket_info.socket[fd].rport = port;
    char *p_str_buff = (char *)malloc(16);
    sprintf(p_str_buff, "%s", ipaddr_ntoa(&(__socket_info.socket[fd].raddr)));
    jerryxx_set_property_string(__socket_info.socket[fd].obj,
                                MSTR_PICO_CYW43_SOCKET_RADDR, p_str_buff);
    free(p_str_buff);
    jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                MSTR_PICO_CYW43_SOCKET_RPORT,
                                __socket_info.socket[fd].rport);
    cyw43_arch_lwip_begin();
    if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
      __socket_info.socket[fd].tcp_pcb =
          tcp_new_ip_type(IP_GET_TYPE(&(__socket_info.socket[fd].raddr)));
      if (!(__socket_info.socket[fd].tcp_pcb)) {
        jerryxx_set_property_number(JERRYXX_GET_THIS,
                                    MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      } else {
        tcp_arg(__socket_info.socket[fd].tcp_pcb,
                &(__socket_info.socket[fd].fd));
        tcp_poll(__socket_info.socket[fd].tcp_pcb, NULL, 0);
        tcp_sent(__socket_info.socket[fd].tcp_pcb, NULL);
        tcp_err(__socket_info.socket[fd].tcp_pcb, NULL);
        tcp_recv(__socket_info.socket[fd].tcp_pcb, __tcp_data_recv_cb);
        err = tcp_connect(
            __socket_info.socket[fd].tcp_pcb, &(__socket_info.socket[fd].raddr),
            __socket_info.socket[fd].rport, __net_client_connect_cb);
      }
    } else { /** UDP */
      __socket_info.socket[fd].udp_pcb =
          udp_new_ip_type(IP_GET_TYPE(&(__socket_info.socket[fd].raddr)));
      if (!(__socket_info.socket[fd].udp_pcb)) {
        jerryxx_set_property_number(JERRYXX_GET_THIS,
                                    MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      } else {
        udp_recv(__socket_info.socket[fd].udp_pcb, __udp_data_recv_cb,
                 &(__socket_info.socket[fd].fd));
        err = udp_connect(__socket_info.socket[fd].udp_pcb,
                          &(__socket_info.socket[fd].raddr),
                          __socket_info.socket[fd].rport);
      }
    }
    cyw43_arch_lwip_end();
    if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
    } else {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
      __socket_info.socket[fd].state = NET_SOCKET_STATE_CONNECTED;
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_STATE,
                                  __socket_info.socket[fd].state);
    }
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  }
  if (JERRYXX_HAS_ARG(3)) {
    jerry_value_t callback = JERRYXX_GET_ARG(3);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_write) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_STRING(1, "data");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(2, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t data = JERRYXX_GET_ARG(1);
  if (km_is_valid_fd(fd) && (((__socket_info.socket[fd].ptcl == NET_SOCKET_DGRAM) &&
                          (__socket_info.socket[fd].state != NET_SOCKET_STATE_CLOSED)) ||
                         ((__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) &&
                          (__socket_info.socket[fd].state >= NET_SOCKET_STATE_CONNECTED)))) {
    jerry_size_t data_str_sz = jerryxx_get_ascii_string_size(data);
    char *data_str = calloc(1, data_str_sz + 1);
    jerryxx_string_to_ascii_char_buffer(data, (uint8_t *)data_str, data_str_sz);
    err_t err = ERR_OK;
    cyw43_arch_lwip_begin();
    if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
      err = tcp_write(__socket_info.socket[fd].tcp_pcb, data_str,
                      data_str_sz, TCP_WRITE_FLAG_COPY);
      if (err == ERR_OK) {
        err = tcp_output(__socket_info.socket[fd].tcp_pcb);
      }
    } else {
      struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, data_str_sz, PBUF_POOL);
      if (p) {
        pbuf_take(p, data_str, data_str_sz);
        err = udp_send(__socket_info.socket[fd].udp_pcb, p);
        pbuf_free(p);
      }
    }
    cyw43_arch_lwip_end();
    if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
    } else {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    }
    free(data_str);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS,
                                MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
  }
  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t callback = JERRYXX_GET_ARG(2);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_close) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(1, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd) && (__socket_info.socket[fd].state != NET_SOCKET_STATE_CLOSED || __socket_info.socket[fd].obj != 0))
    err = __net_socket_close(fd);
  if (err == ERR_OK) {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                0);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  }

  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_shutdown) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_NUMBER(1, "how");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(2, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  int8_t how = JERRYXX_GET_ARG_NUMBER(1);
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd)) {
    if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
      int shut_rx = 0;
      int shut_tx = 0;
      if (how == 0) {
        shut_rx = 1;
      } else if (how == 1) {
        shut_tx = 1;
      } else {
        shut_rx = 1;
        shut_tx = 1;
      }
      cyw43_arch_lwip_begin();
      if (__socket_info.socket[fd].tcp_server_pcb) {
        err = tcp_shutdown(__socket_info.socket[fd].tcp_server_pcb, shut_rx,
                          shut_tx);
      }
      if ((err == ERR_OK) && (__socket_info.socket[fd].tcp_pcb)) {
        err = tcp_shutdown(__socket_info.socket[fd].tcp_pcb, shut_rx, shut_tx);
      }
      cyw43_arch_lwip_end();
    } else {
      /** Nothing to do for UDP */
    }
  } else {
    err = ERANGE;
  }
  if (err != ERR_OK) {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                0);
    jerry_value_t shutdown_js_cb = jerryxx_get_property(
        __socket_info.socket[fd].obj, MSTR_PICO_CYW43_SOCKET_SHUTDOWN_CB);
    if (jerry_value_is_function(shutdown_js_cb)) {
      jerry_value_t this_val = jerry_undefined();
      jerry_call(shutdown_js_cb, this_val, NULL, 0);
      jerry_value_free(this_val);
    }
    jerry_value_free(shutdown_js_cb);
  }
  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t callback = JERRYXX_GET_ARG(2);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_bind) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_STRING(1, "addr");
  JERRYXX_CHECK_ARG_NUMBER(2, "port");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(3, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, addr_str);
  uint16_t port = JERRYXX_GET_ARG_NUMBER(2);
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd) && __socket_info.socket[fd].state == NET_SOCKET_STATE_CLOSED) {
    ip_addr_t laddr;
    cyw43_arch_lwip_begin();
    __cyw43_drv.status_flag &= ~KM_CYW43_STATUS_DNS_DONE;
    err = dns_gethostbyname_addrtype((const char *)addr_str, &(laddr),
                                      __dns_found_cb, &(laddr),
                                      LWIP_DNS_ADDRTYPE_IPV4);
    cyw43_arch_lwip_end();
    if (err == ERR_INPROGRESS) {
      int16_t timeout = 300; // 3 Sec
      while((__cyw43_drv.status_flag & KM_CYW43_STATUS_DNS_DONE) == 0) {
        if (timeout-- <= 0) {
          jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
          return jerry_error_sz(JERRY_ERROR_COMMON,
                                  "DNS response timeout.");
          }
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(100));
#else
        km_delay(10);
#endif
      }
      if (ip4_addr_get_u32(&(laddr)) == 0) {
        jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
        return jerry_error_sz(JERRY_ERROR_COMMON,
                                 "DNS Error: IP is not found.");
      }
      __cyw43_drv.status_flag &= ~KM_CYW43_STATUS_DNS_DONE;
    } else if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      return jerry_error_sz(JERRY_ERROR_COMMON,
                                "DNS Error: DNS access error.");
    }
    __socket_info.socket[fd].lport = port;
    char *p_str_buff = (char *)malloc(16);
    sprintf(p_str_buff, "%s", ipaddr_ntoa(&(laddr)));
    jerryxx_set_property_string(__socket_info.socket[fd].obj,
                                MSTR_PICO_CYW43_SOCKET_LADDR, p_str_buff);
    free(p_str_buff);
    jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                MSTR_PICO_CYW43_SOCKET_LPORT,
                                __socket_info.socket[fd].lport);
    cyw43_arch_lwip_begin();
    if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
      __socket_info.socket[fd].tcp_server_pcb =
          tcp_new_ip_type(IPADDR_TYPE_ANY);
      if (!(__socket_info.socket[fd].tcp_server_pcb)) {
        jerryxx_set_property_number(JERRYXX_GET_THIS,
                                    MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
      } else {
        err = tcp_bind(__socket_info.socket[fd].tcp_server_pcb, NULL,
                       __socket_info.socket[fd].lport);
      }
    } else {
      __socket_info.socket[fd].udp_pcb =
          udp_new_ip_type(IP_GET_TYPE(&(__socket_info.laddr)));
      if (__socket_info.socket[fd].udp_pcb != NULL) {
        err = udp_bind(__socket_info.socket[fd].udp_pcb, &(__socket_info.laddr),
                     __socket_info.socket[fd].lport);
        if (err == ERR_OK) {
          udp_recv(__socket_info.socket[fd].udp_pcb, __udp_data_recv_cb,
                   &(__socket_info.socket[fd].fd));
        }
      }
    }
    cyw43_arch_lwip_end();
    if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
    } else {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
      __socket_info.socket[fd].state = NET_SOCKET_STATE_BIND;
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_STATE,
                                  __socket_info.socket[fd].state);
    }
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  }
  if (JERRYXX_HAS_ARG(3)) {
    jerry_value_t callback = JERRYXX_GET_ARG(3);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_network_listen) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(1, "callback");
  int8_t fd = JERRYXX_GET_ARG_NUMBER(0);
  err_t err = ERR_OK;
  if (km_is_valid_fd(fd) && __socket_info.socket[fd].state == NET_SOCKET_STATE_BIND) {
    if (__socket_info.socket[fd].ptcl == NET_SOCKET_STREAM) {
      __socket_info.socket[fd].tcp_server_pcb =
          tcp_listen(__socket_info.socket[fd].tcp_server_pcb);
      if (__socket_info.socket[fd].tcp_server_pcb) {
        cyw43_arch_lwip_begin();
        tcp_arg(__socket_info.socket[fd].tcp_server_pcb,
                &(__socket_info.socket[fd].fd));
        tcp_accept(__socket_info.socket[fd].tcp_server_pcb,
                   __tcp_server_accept_cb);
        cyw43_arch_lwip_end();
      } else {
        err = ERR_CONN;
      }
    }
    if (err != ERR_OK) {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, -1);
    } else {
      jerryxx_set_property_number(JERRYXX_GET_THIS,
                                  MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
      __socket_info.socket[fd].state = NET_SOCKET_STATE_LISTENING;
      jerryxx_set_property_number(__socket_info.socket[fd].obj,
                                  MSTR_PICO_CYW43_SOCKET_STATE,
                                  __socket_info.socket[fd].state);
    }
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO,
                                -1);
  }
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_NETWORK_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }
  return jerry_undefined();
}

/*
  AP Mode
*/

JERRYXX_FUN(pico_cyw43_wifi_ap_mode) {
  JERRYXX_CHECK_ARG(0, "apInfo");
  JERRYXX_CHECK_ARG_FUNCTION_OPT(1, "callback");
  jerry_value_t ap_info = JERRYXX_GET_ARG(0);
  jerry_size_t len;
  uint8_t *pw_str = NULL;
  ip4_addr_t gw, mask;

  // validate SSID
  jerry_value_t ssid = jerryxx_get_property(ap_info, MSTR_PICO_CYW43_WIFI_APMODE_SSID);
  if (jerry_value_is_string(ssid)) {
    len = jerryxx_get_ascii_string_size(ssid);
    if (len > 32) {
      len = 32;
    }
    jerryxx_string_to_ascii_char_buffer(
        ssid, (uint8_t *)__cyw43_drv.current_ssid, len);
    __cyw43_drv.current_ssid[len] = '\0';
  } else {
    jerry_value_free(ssid);
    return jerry_error_sz(JERRY_ERROR_TYPE, "SSID error");
  }
  jerry_value_free(ssid);

  // validate password
  jerry_value_t password = jerryxx_get_property(ap_info, MSTR_PICO_CYW43_WIFI_APMODE_PASSWORD);
  if (jerry_value_is_string(password)) {
    len = jerryxx_get_ascii_string_size(password);
    if (len < 8) {
      jerry_value_free(password);
      return jerry_error_sz(JERRY_ERROR_COMMON, "PASSWORD need to have at least 8 characters");
    }
    pw_str = (uint8_t *)malloc(len + 1);
    jerryxx_string_to_ascii_char_buffer(password, pw_str, len);
    pw_str[len] = '\0';
  }
  jerry_value_free(password);

  // validate Gateway
  uint8_t *str_buffer = (uint8_t *)malloc(16);
  jerry_value_t gateway = jerryxx_get_property(ap_info, MSTR_PICO_CYW43_WIFI_APMODE_GATEWAY);
  if (jerry_value_is_string(gateway)) {
    len = jerryxx_get_ascii_string_size(gateway);
    jerryxx_string_to_ascii_char_buffer(gateway, str_buffer, len);
  } else {
    const char *gate_ip4 = "192.168.4.1";
    len = 11;
    sprintf((char*)str_buffer, "%s", gate_ip4);
  }
  str_buffer[len] = '\0';
  if (ipaddr_aton((const char *)str_buffer, &(gw)) == false) {
    free(pw_str);
    free(str_buffer);
    jerry_value_free(gateway);
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "Can't decode Gateway IP Address");
  }
  jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_APMODE_GATEWAY,
                              (char *)str_buffer);
  sprintf(__cyw43_drv.ap_gateway_ip4, "%s", str_buffer);
  jerry_value_free(gateway);

  // validate subnet mask
  jerry_value_t subnet_mask = jerryxx_get_property(ap_info, MSTR_PICO_CYW43_WIFI_APMODE_SUBNET_MASK);
  if (jerry_value_is_string(subnet_mask)) {
    len = jerryxx_get_ascii_string_size(subnet_mask);
    jerryxx_string_to_ascii_char_buffer(subnet_mask, str_buffer, len);
  } else {
    const char *subnet_ip4 = "255.255.255.0";
    len = 13;
    sprintf((char *)str_buffer, "%s", subnet_ip4);
  }
  str_buffer[len] = '\0';
  if (ipaddr_aton((const char *)str_buffer, &(mask)) == false) {
    free(pw_str);
    free(str_buffer);
    jerry_value_free(subnet_mask);
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "Can't decode Subnet Mask");
  }
  jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_APMODE_SUBNET_MASK,
                              (char *)str_buffer);
  free(str_buffer);
  jerry_value_free(subnet_mask);

  // init driver
  int err = __cyw43_init();
  if (err == 0) {
    cyw43_arch_enable_ap_mode((char *) __cyw43_drv.current_ssid, (char *) pw_str, CYW43_AUTH_WPA2_AES_PSK);
    free(pw_str);
    // start DHCP server
    err = dhcp_server_init(&dhcp_server, &gw, &mask);
    if (err != 0) {
      km_cyw43_deinit();
      __cyw43_init();
    } else {
      __cyw43_drv.status_flag |= KM_CYW43_STATUS_AP_MODE;
    }
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO,
                              err);
  // call callback
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t js_cb = jerry_value_copy(callback);
    jerry_value_t errno = jerryxx_get_property_number(
        JERRYXX_GET_THIS, MSTR_PICO_CYW43_WIFI_ERRNO, 0);
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args_p[1] = {errno};
    jerry_call(js_cb, this_val, args_p, 1);
    jerry_value_free(errno);
    jerry_value_free(this_val);
    jerry_value_free(js_cb);
  }

  return jerry_undefined();
}

JERRYXX_FUN(pico_cyw43_wifi_disable_ap_mode) {
  // verify if AP_mode is enabled
  if ((__cyw43_drv.status_flag & KM_CYW43_STATUS_AP_MODE) == 0) {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "WiFi AP_mode is not enabled.");
  }
  __cyw43_drv.status_flag &= ~KM_CYW43_STATUS_AP_MODE;
  // deinit DHCP server
  dhcp_server_deinit(&dhcp_server);
  km_cyw43_deinit();
  // init the WiFi chip
  if (__cyw43_init()) {
    return jerry_exception_value(create_system_error(EAGAIN), true);
  }
  return jerry_undefined();
}

// Function to get the MAC address of connected clients
JERRYXX_FUN(pico_cyw43_wifi_ap_get_stas) {
  // verify if AP_mode is enabled
  if ((__cyw43_drv.status_flag & KM_CYW43_STATUS_AP_MODE) == 0) {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "WiFi AP_mode is not enabled.");
  }

  int num_stas;
  int max_stas;
  // get max stas
  cyw43_wifi_ap_get_max_stas(&cyw43_state, &max_stas);
  // declare
  uint8_t *macs = (uint8_t*)malloc(max_stas * 6);
  //uint8_t macs[32 * 6];
  cyw43_wifi_ap_get_stas(&cyw43_state, &num_stas, macs);
  jerry_value_t MAC_array = jerry_array (num_stas);
  char **mac_strs = (char **)malloc(num_stas * sizeof(char *));
  for (int i = 0; i < num_stas; i++) {
    mac_strs[i] = (char *)malloc(MAC_STR_LENGTH * sizeof(char));
    sprintf(mac_strs[i], "%02x:%02x:%02x:%02x:%02x:%02x", macs[i*6], macs[i*6+1], macs[i*6+2], macs[i*6+3], macs[i*6+4], macs[i*6+5]);
    // add to the array
    jerry_value_t prop = jerry_string_sz((const char *)mac_strs[i]);
    jerry_value_free(jerry_object_set_index(MAC_array, i, prop));
    jerry_value_free(prop);
    free(mac_strs[i]);
  }
  // deallocate memory
  free(mac_strs);
  free(macs);
  // return the list of macs
  return MAC_array;
}

jerry_value_t module_pico_cyw43_init() {
  /* PICO_CYW43 class */
  jerry_value_t pico_cyw43_ctor =
      jerry_function_external(pico_cyw43_ctor_fn);
  jerry_value_t prototype = jerry_object();
  jerryxx_set_property(pico_cyw43_ctor, "prototype", prototype);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_GETGPIO,
                                pico_cyw43_get_gpio);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_PUTGPIO,
                                pico_cyw43_put_gpio);
  jerry_value_free(prototype);

  jerry_value_t pico_cyw43_wifi_ctor =
      jerry_function_external(pico_cyw43_wifi_ctor_fn);
  jerry_value_t wifi_prototype = jerry_object();
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
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_FN,
                                pico_cyw43_wifi_ap_mode);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_GET_STAS_FN,
                                pico_cyw43_wifi_ap_get_stas);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_DISABLE_FN,
                                pico_cyw43_wifi_disable_ap_mode);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_DRV_FN,
                                pico_cyw43_wifi_ap_mode);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_GET_STAS_DRV_FN,
                                pico_cyw43_wifi_ap_get_stas);
  jerryxx_set_property_function(wifi_prototype,
                                MSTR_PICO_CYW43_WIFI_APMODE_DISABLE_DRV_FN,
                                pico_cyw43_wifi_disable_ap_mode);
  jerry_value_free(wifi_prototype);

  jerry_value_t pico_cyw43_network_ctor =
      jerry_function_external(pico_cyw43_network_ctor_fn);
  jerry_value_t network_prototype = jerry_object();
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
  jerry_value_free(network_prototype);

  /* pico_cyw43 module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43, pico_cyw43_ctor);
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43_WIFI,
                       pico_cyw43_wifi_ctor);
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43_NETWORK,
                       pico_cyw43_network_ctor);
  jerry_value_free(pico_cyw43_ctor);
  jerry_value_free(pico_cyw43_wifi_ctor);
  jerry_value_free(pico_cyw43_network_ctor);

  return exports;
}
