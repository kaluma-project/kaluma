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

#include <stdlib.h>
#include "jerryscript.h"
#include "jerryxx.h"
#include "wifi_magic_strings.h"
#include "ieee80211.h"
#include "io.h"

static void scan_cb(io_ieee80211_handle_t *, int count, ieee80211_scan_info_t* records);
static void assoc_cb(io_ieee80211_handle_t *);
static void connect_cb(io_ieee80211_handle_t *);
static void disconnect_cb(io_ieee80211_handle_t *);

JERRYXX_FUN(wifi_ctor_fn) {
    io_ieee80211_handle_t *handle = malloc(sizeof(io_ieee80211_handle_t));
    io_ieee80211_init(handle);
    handle->scan_js_cb = jerry_create_null();
    handle->this_val = jerry_acquire_value(JERRYXX_GET_THIS);

    io_ieee80211_start(handle, scan_cb, assoc_cb, connect_cb, disconnect_cb);
    jerryxx_set_property_number(JERRYXX_GET_THIS, "handle_id", handle->base.id);

    return jerry_create_undefined();
}

JERRYXX_FUN(wifi_reset_fn) {
    JERRYXX_CHECK_ARG_FUNCTION(0, "cb");
    jerry_value_t callback = JERRYXX_GET_ARG(0);
    int ret = ieee80211_reset();
    jerry_value_t arg = jerry_create_number(ret);
    if (jerry_value_is_function(callback)) {
        jerry_value_t args[1] = { arg };

        jerry_value_t ret_val = jerry_call_function(callback, jerry_create_undefined(), args, 1);
        if (jerry_value_is_error(ret_val)) {
            jerryxx_print_error(ret_val, true);
        }
        jerry_release_value(ret_val);
        jerry_release_value(args);
    }
    jerry_release_value(arg);
    return jerry_create_undefined();
}

//===============================================
// SCAN
//#pragma region SCAN
static jerry_value_t ap_list_to_jobject(const ieee80211_scan_info_t* ap_list, int count)
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

static void scan_cb(io_ieee80211_handle_t *handle, int count, ieee80211_scan_info_t* records)
{
    if (jerry_value_is_function(handle->scan_js_cb)) {
        jerry_value_t err_js = jerry_create_number(0);
        jerry_value_t records_js = ap_list_to_jobject(records, count);
        jerry_value_t this_val = handle->this_val;
        jerry_value_t args_p[2] = { err_js, records_js };
        jerry_value_t ret = jerry_call_function(handle->scan_js_cb, this_val, args_p, 2);
        if (jerry_value_is_error(ret)) {
            jerryxx_print_error(ret, true);
        }
        jerry_release_value(ret);
        jerry_release_value(this_val);
        jerry_release_value(records_js);
        jerry_release_value(err_js);
    }
}

JERRYXX_FUN(wifi_scan_fn) {
    JERRYXX_CHECK_ARG_FUNCTION(0, "cb");
    jerry_value_t callback = JERRYXX_GET_ARG(0);

    int handle_id = jerryxx_get_property_number(this_val, "handle_id", 0);
    io_ieee80211_handle_t* handle = io_ieee80211_get_by_id(handle_id);
    jerry_release_value(handle->scan_js_cb);
    handle->scan_js_cb = jerry_acquire_value(callback);

    int ret = ieee80211_scan();
    return jerry_create_number(ret);
}

//===============================================
// ASSOC
static void assoc_cb(io_ieee80211_handle_t *handle)
{
    jerry_value_t this_val = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(this_val, "assoc_cb");
    jerry_value_t args_p[0];

    if (jerry_value_is_function(callback)) {
        jerry_value_t ret = jerry_call_function(callback, this_val, args_p, 0);
        if (jerry_value_is_error(ret)) {
            jerryxx_print_error(ret, true);
        }
        jerry_release_value(ret);
    }

    jerry_release_value(callback);
}


//===============================================
// CONNECT
static void connect_cb(io_ieee80211_handle_t *handle)
{
    jerry_value_t this_val = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(this_val, "connect_cb");

    if (jerry_value_is_function(callback)) {
        jerry_value_t err_js = jerry_create_number(0);
        jerry_value_t arg = jerry_create_undefined();
        jerry_value_t args_p[2] = { err_js, arg };
        jerry_value_t ret = jerry_call_function(callback, this_val, args_p, 2);
        if (jerry_value_is_error(ret)) {
            jerryxx_print_error(ret, true);
        }
        jerry_release_value(ret);
        jerry_release_value(arg);
        jerry_release_value(err_js);
    }

    jerry_release_value(callback);
}


JERRYXX_FUN(wifi_connect_fn) {
    JERRYXX_CHECK_ARG_OBJECT(0, "connectInfo");
    JERRYXX_CHECK_ARG_FUNCTION(1, "cb");

    // set connectInfo
    jerry_value_t connectInfo = JERRYXX_GET_ARG(0);
    JERRYXX_GET_PROPERTY_STRING_AS_CHAR(connectInfo, ssid);
    JERRYXX_GET_PROPERTY_STRING_AS_CHAR(connectInfo, password);

    // connect
    int retVal = ieee80211_connect((const char*)ssid, (const char*)password);

    // call callback
    jerry_value_t callback = JERRYXX_GET_ARG(1);
    jerry_value_t err_js = jerry_create_number(retVal);
    jerry_value_t args[1] = { err_js };
    jerry_value_t ret = jerry_call_function(callback, this_val, args, 1);
    if (jerry_value_is_error(ret)) {
        jerryxx_print_error(ret, true);
    }
    jerry_release_value(ret);
    jerry_release_value(err_js);
    return retVal;
}

//===============================================
// DISCONNECT
static void disconnect_cb(io_ieee80211_handle_t *handle)
{
    jerry_value_t this_val = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(this_val, "disconnect_cb");

    if (jerry_value_is_function(callback)) {
        jerry_value_t ret = jerry_call_function(callback, this_val, NULL, 0);
        if (jerry_value_is_error(ret)) {
            jerryxx_print_error(ret, true);
        }
        jerry_release_value(ret);
    }

    jerry_release_value(callback);
}


JERRYXX_FUN(wifi_disconnect_fn) {
    JERRYXX_CHECK_ARG_FUNCTION(0, "cb");

    int retVal = ieee80211_disconnect();

    jerry_value_t callback = JERRYXX_GET_ARG(0);
    jerry_value_t err_js = jerry_create_number(retVal);
    jerry_value_t args[1] = { err_js };
    jerry_value_t ret = jerry_call_function(callback, this_val, args, 1);
    if (jerry_value_is_error(ret)) {
        jerryxx_print_error(ret, true);
    }
    jerry_release_value(ret);
    jerry_release_value(err_js);

    return retVal;
}

JERRYXX_FUN(wifi_get_connection_fn) {
    ieee80211_scan_info_t ap_info;
    int ret = ieee80211_sta_get_ap_info(&ap_info);

    jerry_value_t jsinfo = jerry_create_object();

    jerry_value_t bssid = jerry_create_arraybuffer(6);
    jerry_arraybuffer_write(bssid, 0, ap_info.bssid, 6);
    jerryxx_set_property(jsinfo, "bssid", bssid);
    jerry_release_value (bssid);

    jerry_value_t ssid = jerry_create_arraybuffer(33);
    jerry_arraybuffer_write(ssid, 0, ap_info.ssid, 33);
    jerryxx_set_property(jsinfo, "ssid", ssid);
    jerry_release_value (ssid);

    jerryxx_set_property_number(jsinfo, "primary_channel", ap_info.primary_channel);
    jerryxx_set_property_number(jsinfo, "rssi", ap_info.rssi);
    jerryxx_set_property_number(jsinfo, "authmode", ap_info.authmode);
    jerryxx_set_property_number(jsinfo, "cipher", ap_info.cipher);

    return jsinfo;
}

jerry_value_t module_wifi_init() {
    /* IEEE80211 class */
    jerry_value_t wifi_ctor = jerry_create_external_function(wifi_ctor_fn);
    jerry_value_t wifi_prototype = jerry_create_object();
    jerryxx_set_property_function(wifi_prototype, MSTR_WIFI_RESET, wifi_reset_fn);
    jerryxx_set_property_function(wifi_prototype, MSTR_WIFI_SCAN, wifi_scan_fn);
    jerryxx_set_property_function(wifi_prototype, MSTR_WIFI_CONNECT, wifi_connect_fn);
    jerryxx_set_property_function(wifi_prototype, MSTR_WIFI_DISCONNECT, wifi_disconnect_fn);
    jerryxx_set_property_function(wifi_prototype, MSTR_WIFI_GET_CONNECTION, wifi_get_connection_fn);
    jerryxx_set_property(wifi_ctor, "prototype", wifi_prototype);
    jerry_release_value (wifi_prototype);

    /* uart module exports */
    jerry_value_t exports = jerry_create_object();
    jerryxx_set_property(exports, MSTR_WIFI_WIFI_, wifi_ctor);
    jerry_release_value (wifi_ctor);

    return exports;
}
