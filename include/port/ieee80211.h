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

#ifndef __KM_IEEE80211_H
#define __KM_IEEE80211_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KM_IEEE80211_SECURITY_OPEN 0x00
#define KM_IEEE80211_SECURITY_WEP 0x01
#define KM_IEEE80211_SECURITY_WPA 0x02
#define KM_IEEE80211_SECURITY_PSK 0x04
#define KM_IEEE80211_SECURITY_WPA2 0x08
#define KM_IEEE80211_SECURITY_WPA2_EAP 0x10

typedef struct {
  char *bssid;
  char *ssid;
  uint16_t security;
  int rssi;
  int channel;
} km_ieee80211_scan_result_t;

/**
 * Initialize
 */
void km_ieee80211_init();

/**
 * Cleanup
 */
void km_ieee80211_cleanup();

/**
 * Scan wireless networks.
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_ieee80211_scan();

/**
 * Connect to a wireless network
 * @param ssid
 * @param password
 * @param security OPEN or multiple of WEP, WPA, PSK, WPA2, WPA2-EAP
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_ieee80211_connect(char *ssid, char *password, int security);

/**
 * Disconnect from the wireless network
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_ieee80211_disconnect();

#endif /* __KM_IEEE80211_H */
