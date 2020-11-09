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

#pragma once

#include <stdint.h>

typedef enum {
  OPEN = 0,
  WEP = 1,
  WPA_PSK = 2,
  WPA2_PSK = 3,
  WPA_WPA2_PSK = 4,
  WPA2_ENTERPRISE = 5,
  WPA3_PSK = 6,
  WPA2_WPA3_PSK = 7
} ieee80211_auth_t;

typedef enum {
  NONE = 0,
  WEP40 = 1,
  WEP104 = 2,
  TKIP = 3,
  CCMP = 4,
  TKIP_CCMP = 5,
  AES_CMAC128 = 6
} ieee80211_cipher_t;

typedef struct {
  uint8_t bssid[6];
  uint8_t ssid[33];
  uint8_t primary_channel;
  int8_t rssi;
  ieee80211_auth_t authmode;
  ieee80211_cipher_t cipher;
} ieee80211_scan_info_t;


/**
 * Initialize the device driver.
 */
int ieee80211_init();

/**
 * Cleanup the device driver.
 */
int ieee80211_cleanup();


int ieee80211_reset();

/**
 * Start wifi network service and connect to AP(ssid) 
 */
int ieee80211_connect(const char* ssid, const char* password);

/**
 * Stop wifi network service
 */
int ieee80211_disconnect();

/**
 * Start scanning AP
 */
int ieee80211_scan();

/**
 * Get AP information which is connected.
 */
int ieee80211_sta_get_ap_info(ieee80211_scan_info_t* ap_info);


typedef enum {
  IEEE80211_EVENT_SCAN = 0,
  IEEE80211_EVENT_ASSOC = 1,
  IEEE80211_EVENT_CONNECT = 2,
  IEEE80211_EVENT_DISCONNECT = 3,
} ieee80211_event_code_t;

typedef struct {
  ieee80211_event_code_t code;
  int count;
  ieee80211_scan_info_t* records;
} ieee80211_event_scan_t;

typedef struct {
  ieee80211_event_code_t code;
} ieee80211_event_assoc_t;

typedef struct {
  ieee80211_event_code_t code;
} ieee80211_event_connect_t;

typedef struct {
  ieee80211_event_code_t code;
} ieee80211_event_disconnect_t;

typedef union {
  ieee80211_event_code_t code;
  ieee80211_event_scan_t scan;
  ieee80211_event_assoc_t assoc;
  ieee80211_event_connect_t connect;
  ieee80211_event_disconnect_t disconnect;
} ieee80211_event_t;

/**
 * Get WIFI event in message queue
 * 0 : if success, -1 : else
 */
int ieee80211_get_event(ieee80211_event_t* out_event);
