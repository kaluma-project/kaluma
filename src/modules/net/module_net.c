/* Copyright (c) 2017 Kalamu
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
#include "net_magic_strings.h"
#include "io.h"
#include <esp_log.h>
#include <string.h>

#define TAG ("MODULE_NET")

static void net_on_connect(io_tcp_handle_t *handle);

static void net_on_disconnect(io_tcp_handle_t *handle);

static void net_on_read(io_tcp_handle_t *handle, const char *message, int len);


static void PrintJsType(jerry_value_t obj, const char *name) {
    if (jerry_value_is_null(obj)) {
        ESP_LOGE(TAG, "%s is null", name);
    }
    if (jerry_value_is_undefined(obj)) {
        ESP_LOGE(TAG, "%s is undefined", name);
    }
    if (jerry_value_is_function(obj)) {
        ESP_LOGE(TAG, "%s is function", name);
    }
    if (jerry_value_is_object(obj)) {
        ESP_LOGE(TAG, "%s is object", name);
    }
    if (jerry_value_is_constructor(obj)) {
        ESP_LOGE(TAG, "%s is constructor", name);
    }
    if (jerry_value_is_symbol(obj)) {
        ESP_LOGE(TAG, "%s is symbol", name);
    }
    if (jerry_value_is_array(obj)) {
        ESP_LOGE(TAG, "%s is array", name);
    }
    if (jerry_value_is_number(obj)) {
        ESP_LOGE(TAG, "%s is number", name);
    }
    if (jerry_value_is_string(obj)) {
        ESP_LOGE(TAG, "%s is string", name);
    }
    if (jerry_value_is_boolean(obj)) {
        ESP_LOGE(TAG, "%s is boolean", name);
    }
    if (jerry_value_is_error(obj)) {
        ESP_LOGE(TAG, "%s is error", name);
    }
}

JERRYXX_FUN(net_connect_fn) {
    ESP_LOGD(TAG, "net_connect_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_STRING(1, "addr");
    JERRYXX_CHECK_ARG_NUMBER(2, "port");
    JERRYXX_CHECK_ARG_FUNCTION(3, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    JERRYXX_GET_ARG_STRING_AS_CHAR(1, addr);
    int port = JERRYXX_GET_ARG_NUMBER(2);
    jerry_value_t callback = JERRYXX_GET_ARG(3);

    io_tcp_handle_t *handle = io_tcp_get_by_fd(fd);
    if (handle == NULL) {
        return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Invalid argument 'fd'");
    }
    jerry_value_t obj = handle->this_val;
    jerryxx_set_property(obj, "connect_cb", callback);

    io_tcp_connect(handle, addr, port);
    return jerry_create_undefined();
}


static void net_on_connect(io_tcp_handle_t *handle) {
    ESP_LOGD(TAG, "net_on_connect");
    jerry_value_t thiz = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(thiz, "connect_cb");
    if (!jerry_value_is_function(callback)) {
        ESP_LOGE(TAG, "callback is NOT function");
        return;
    }
    jerry_call_function(callback, thiz, NULL, 0);
}

JERRYXX_FUN(net_write_fn) {
    ESP_LOGD(TAG, "net_write_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_STRING(1, "data");
    JERRYXX_CHECK_ARG_FUNCTION(2, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    JERRYXX_GET_ARG_STRING_AS_CHAR(1, data);
    jerry_value_t callback = JERRYXX_GET_ARG(2);

    io_tcp_handle_t *handle = io_tcp_get_by_fd(fd);
    if (handle == NULL) {
        return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Invalid argument 'fd'");
    }

    io_tcp_send(handle, data, strlen(data));
    return jerry_create_undefined();
}

JERRYXX_FUN(net_close_fn) {
    ESP_LOGD(TAG, "net_close_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_FUNCTION(1, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    jerry_value_t callback = JERRYXX_GET_ARG(1);

    io_tcp_handle_t *handle = io_tcp_get_by_fd(fd);
    if (handle == NULL) {
        return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Invalid argument 'fd'");
    }

    io_tcp_close(handle);
    return jerry_create_undefined();
}

JERRYXX_FUN(net_shutdown_fn) {
    ESP_LOGD(TAG, "net_shutdown_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_NUMBER(1, "how");
    JERRYXX_CHECK_ARG_FUNCTION(2, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    int how = JERRYXX_GET_ARG_NUMBER(1);
    jerry_value_t callback = JERRYXX_GET_ARG(2);

    io_tcp_handle_t *handle = io_tcp_get_by_fd(fd);
    if (handle == NULL) {
        return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Invalid argument 'fd'");
    }

    io_tcp_close(handle);
    return jerry_create_undefined();
}

JERRYXX_FUN(net_bind_fn) {
    ESP_LOGD(TAG, "net_bind_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_STRING(1, "addr");
    JERRYXX_CHECK_ARG_NUMBER(2, "port");
    JERRYXX_CHECK_ARG_FUNCTION(3, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    JERRYXX_GET_ARG_STRING_AS_CHAR(1, addr);
    int port = JERRYXX_GET_ARG_NUMBER(2);
    jerry_value_t callback = JERRYXX_GET_ARG(3);

    // TODO
    return jerry_create_undefined();
}

JERRYXX_FUN(net_listen_fn) {
    ESP_LOGD(TAG, "net_listen_fn");

    JERRYXX_CHECK_ARG_NUMBER(0, "fd");
    JERRYXX_CHECK_ARG_FUNCTION(1, "callback");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    jerry_value_t callback = JERRYXX_GET_ARG(1);

    // TODO
    return jerry_create_undefined();
}

JERRYXX_FUN(net_socket_fn) {
    ESP_LOGD(TAG, "net_socket_fn");
    JERRYXX_CHECK_ARG_STRING(1, "protocol");
    JERRYXX_GET_ARG_STRING_AS_CHAR(1, protocol);

    if (strcmp(protocol, "STREAM") != 0) {
        return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "NOT SUPPORTED");
    }
    int fd = kameleon_tcp_socket();
    jerry_value_t undefined = jerry_create_undefined();
    jerry_value_t obj = jerry_create_object();

    jerryxx_set_property_number(obj, "fd", fd);
    jerryxx_set_property(obj, "ptcl", undefined);
    jerryxx_set_property(obj, "state", undefined);
    jerryxx_set_property(obj, "laddr", undefined);
    jerryxx_set_property(obj, "lport", undefined);
    jerryxx_set_property(obj, "raddr", undefined);
    jerryxx_set_property(obj, "rport", undefined);

    io_tcp_handle_t *handle = malloc(sizeof(io_tcp_handle_t));
    io_tcp_init(handle);
    handle->this_val = jerry_acquire_value(obj);
    handle->fd = fd;
    io_tcp_start(handle, net_on_connect, net_on_disconnect, net_on_read);

    return jerry_create_number(fd);
}

static void net_on_disconnect(io_tcp_handle_t *handle) {
    ESP_LOGD(TAG, "net_on_disconnect");
    jerry_value_t thiz = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(thiz, "shutdown_cb");
    if (!jerry_value_is_function(callback)) {
        ESP_LOGE(TAG, "callback is NOT function");
        return;
    }
    jerry_call_function(callback, thiz, NULL, 0);
}

static void net_on_read(io_tcp_handle_t *handle, const char *message, int len) {
    ESP_LOGD(TAG, "net_on_read");
    jerry_value_t thiz = handle->this_val;
    jerry_value_t callback = jerryxx_get_property(thiz, "read_cb");
    if (!jerry_value_is_function(callback)) {
        ESP_LOGE(TAG, "callback is NOT function");
        return;
    }
    jerry_value_t strMsg = jerry_create_string((const jerry_char_t *) message);
    jerry_value_t args[] = {strMsg};
    jerry_call_function(callback, thiz, args, 1);
}

JERRYXX_FUN(net_get_fn) {
    ESP_LOGD(TAG, "net_get_fn");
    JERRYXX_CHECK_ARG_NUMBER(0, "fd");

    int fd = JERRYXX_GET_ARG_NUMBER(0);
    io_tcp_handle_t *handle = io_tcp_get_by_fd(fd);
    if (!handle) {
        return jerry_create_undefined();
    }
    return jerry_acquire_value(handle->this_val);
}


jerry_value_t module_net_init() {
    /* NET class */
    ESP_LOGD(TAG, "module_net_init");

    // create object
    jerry_value_t undefined = jerry_create_undefined();
    jerry_value_t obj = jerry_create_object();
    jerryxx_set_property_function(obj, "connect", net_connect_fn);
    jerryxx_set_property_function(obj, "write", net_write_fn);
    jerryxx_set_property_function(obj, "close", net_close_fn);
    jerryxx_set_property_function(obj, "shutdown", net_shutdown_fn);
    jerryxx_set_property_function(obj, "bind", net_bind_fn);
    jerryxx_set_property_function(obj, "listen", net_listen_fn);
    jerryxx_set_property_function(obj, "socket", net_socket_fn);
    jerryxx_set_property_function(obj, "get", net_get_fn);

    jerryxx_set_property_number(obj, "errno", 0);
    jerryxx_set_property(obj, "mac", undefined);
    jerryxx_set_property(obj, "ip", undefined);
    return obj;
}
