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

#include "err.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"
#include "net_magic_strings.h"
#include "socket.h"

static void socket_connect_cb(km_io_socket_handle_t *handle) {}

/**
 * SocketNative() constructor
 */
JERRYXX_FUN(socket_ctor_fn) {
  int fd = km_socket_create(KM_SOCKET_DOMAIN_TYPE_AF_INET,
                            KM_SOCKET_PROTOCOL_TYPE_STREAM);
  if (fd < 0) {
    return jerry_create_error_from_value(create_system_error(fd), true);
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, "fd", fd);

  return jerry_create_undefined();
}

/**
 * SocketNative.prototype.connect(host, port)
 * args:
 * - host {string}
 * - port {number}
 */
JERRYXX_FUN(socket_connect_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "host");
  JERRYXX_CHECK_ARG_NUMBER(1, "port");
  JERRYXX_CHECK_ARG_FUNCTION(2, "callback");

  // read parameters
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, host);
  uint16_t port = (int)JERRYXX_GET_ARG_NUMBER(1);
  jerry_value_t callback = JERRYXX_GET_ARG(2);

  // setup socket handle
  km_io_socket_handle_t *handle = malloc(sizeof(km_io_socket_handle_t));
  km_io_socket_init(handle);
  jerryxx_set_property_number(JERRYXX_GET_THIS, "handle_id", handle->base.id);

  uint32_t handle_id =
      jerryxx_get_property_number(JERRYXX_GET_THIS, "handle_id", 0);
  km_io_socket_handle_t *handle = km_io_socket_get_by_id(handle_id);

  // socket connect
  handle->connect_js_cb = callback;
  km_socket_address_t address;
  address.host = host;
  address.port = port;
  int ret = km_io_socket_connect(handle, &address, socket_connect_cb);
  if (ret < 0) {
    return jerry_create_error_from_value(create_system_error(ret), true);
  }
  return jerry_create_undefined();
}

/**
 * Initialize 'net' module and return exports
 */
jerry_value_t module_net_init() {
  /* SocketNative class */
  jerry_value_t socket_ctor = jerry_create_external_function(socket_ctor_fn);
  jerry_value_t socket_prototype = jerry_create_object();
  jerryxx_set_property(socket_ctor, MSTR_PROTOTYPE, socket_prototype);
  jerryxx_set_property_function(socket_prototype, MSTR_NET_CONNECT,
                                socket_connect_fn);
  jerry_release_value(socket_prototype);

  /* net module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_NET_SOCKET_NATIVE, socket_ctor);
  jerry_release_value(socket_ctor);
  return exports;
}
