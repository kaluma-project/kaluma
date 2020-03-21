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
#include "uart_magic_strings.h"
#include "uart.h"
#include "io.h"

#define UART_DEFAULT_BAUDRATE 9600
#define UART_DEFAULT_BITS 8
#define UART_DEFAULT_PARITY UART_PARITY_TYPE_NONE
#define UART_DEFAULT_STOP 1
#define UART_DEFAULT_FLOW UART_FLOW_NONE
#define UART_DEFAULT_BUFFERSIZE 1024

static int uart_available_cb(io_uart_handle_t *handle) {
  uint8_t port = handle->port;
  int condition = handle->temp;
  int len = uart_available(port);
  if (condition == 0) {
    return len;
  } else if (condition > 0) {
    if (len >= condition) {
      return condition;
    }
  } else { /* condition < 0 */
    uint8_t endchar = (condition * -1);
    for (int i = 0; i < len; i++) {
      uint8_t ch = uart_available_at(port, i);
      if (ch == endchar) {
        return (i + 1);
      }
    }
  }
  return 0;
}

static void uart_read_cb(io_uart_handle_t *handle, uint8_t *buf, size_t len) {
  if (jerry_value_is_function(handle->read_js_cb)) {
    jerry_value_t array_buffer = jerry_create_arraybuffer(len);
    jerry_arraybuffer_write(array_buffer, 0, buf, len);
    jerry_value_t array = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, array_buffer);
    jerry_release_value(array_buffer);
    jerry_value_t this_val = jerry_create_undefined();
    jerry_value_t args_p[1] = { array };
    jerry_value_t ret_val = jerry_call_function(handle->read_js_cb, this_val, args_p, 1);
    if (jerry_value_is_error(ret_val)) {
      jerryxx_print_error(ret_val, true);
    }
    jerry_release_value(ret_val);
    jerry_release_value(this_val);
    jerry_release_value(array);
  }
}

static void uart_close_cb(io_handle_t *handle) {
  free(handle);
}

/**
 * uart_native constructor
 * args:
 *   port {number}
 *   options {Object}
 *   callback (function(data))
 */
JERRYXX_FUN(uart_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "port");
  JERRYXX_CHECK_ARG_OBJECT(1, "options");
  JERRYXX_CHECK_ARG_FUNCTION(2, "callback");

  // read parameters
  uint8_t port = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t options = JERRYXX_GET_ARG(1);
  jerry_value_t callback = JERRYXX_GET_ARG(2);
  uint32_t baudrate = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BAUDRATE, UART_DEFAULT_BAUDRATE);
  uint32_t bits = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BITS, UART_DEFAULT_BITS);
  uint32_t parity = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_PARITY, UART_DEFAULT_PARITY);
  uint32_t stop = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_STOP, UART_DEFAULT_STOP);
  uint32_t flow = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_FLOW, UART_DEFAULT_FLOW);
  uint32_t buffer_size = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BUFFERSIZE, UART_DEFAULT_BUFFERSIZE);
  jerry_value_t data_event = jerryxx_get_property(options, MSTR_UART_DATAEVENT);

  // initialize the port
  int ret = uart_setup(port, baudrate, bits, parity, stop, flow, buffer_size);
  if (ret == UARTPORT_ERROR) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "UART port setup error.");
  }

  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_PORT, port);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BAUDRATE, baudrate);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BITS, bits);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_PARITY, parity);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_STOP, stop);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_FLOW, flow);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BUFFERSIZE, buffer_size);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_DATAEVENT, data_event);
  jerryxx_set_property(JERRYXX_GET_THIS, "callback", callback);

  // setup io handle
  io_uart_handle_t *handle = malloc(sizeof(io_uart_handle_t));
  io_uart_init(handle);
  handle->read_js_cb = jerry_acquire_value(callback);
  int condition = 0;
  if (jerry_value_is_number(data_event)) {
    condition = (int) jerry_get_number_value(data_event);
  } else if (jerry_value_is_string(data_event)) {
    jerry_size_t sz = jerry_get_string_size(data_event);
    uint8_t sbuf[sz];
    jerry_string_to_char_buffer(data_event, sbuf, sz);
    if (sz > 0) {
      uint8_t endchar = sbuf[0];
      condition = (endchar * -1);
    }
  }
  handle->temp = condition;
  jerryxx_set_property_number(JERRYXX_GET_THIS, "handle_id", handle->base.id);
  io_uart_read_start(handle, port, uart_available_cb, uart_read_cb);

  return jerry_create_undefined();
}

JERRYXX_FUN(uart_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "count");

  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t count = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 1);

  // check this.port
  jerry_value_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);
  jerry_release_value(port_value);

  // write data to the port
  int ret = UARTPORT_ERROR;
  if (jerry_value_is_typedarray(data) && 
      jerry_get_typedarray_type(data) == JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t *buf = jerry_get_arraybuffer_pointer(array_buffer);
    for (int c = 0; c < count; c++) {
      ret = uart_write(port, buf, len);
      if (ret < 0) break;
    }
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t buf[len];
    jerryxx_string_to_ascii_char_buffer(data, buf, len);
    for (int c = 0; c < count; c++) {
      ret = uart_write(port, buf, len);
      if (ret < 0) break;
    }
  } else {
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "The data argument must be Uint8Array or string.");
  }
  if (ret == UARTPORT_ERROR)
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to write data to UART port.");
  else
    return jerry_create_number(ret);
}


/**
 * UART.prototype.close() function
 */
JERRYXX_FUN(uart_close_fn) {
  // check this.port
  jerry_value_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);
  jerry_release_value(port_value);

  // close the port
  int ret = uart_close(port);
  if (ret == UARTPORT_ERROR) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to close UART port.");
  }

  // delete this.port
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_UART_PORT);

  // close io handle
  uint32_t handle_id = jerryxx_get_property_number(JERRYXX_GET_THIS, "handle_id", 0);
  io_uart_handle_t *handle = io_uart_get_by_id(handle_id);
  if (handle != NULL) {
    jerry_release_value(handle->read_js_cb);
    io_uart_read_stop(handle);
    io_handle_close((io_handle_t *) handle, uart_close_cb);
  }
  jerryxx_delete_property(JERRYXX_GET_THIS, "handle_id");

  return jerry_create_undefined();
}

/**
 * Initialize 'uart' module and return exports
 */
jerry_value_t module_uart_init() {
  /* UART class */
  jerry_value_t uart_ctor = jerry_create_external_function(uart_ctor_fn);
  jerry_value_t uart_prototype = jerry_create_object();
  jerryxx_set_property(uart_ctor, "prototype", uart_prototype);
  jerryxx_set_property_function(uart_prototype, MSTR_UART_WRITE, uart_write_fn);
  jerryxx_set_property_function(uart_prototype, MSTR_UART_CLOSE, uart_close_fn);
  jerry_release_value (uart_prototype);

  /* uart module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_UART_UART, uart_ctor);
  jerryxx_set_property_number(exports, MSTR_UART_PARITY_NONE, UART_PARITY_TYPE_NONE);
  jerryxx_set_property_number(exports, MSTR_UART_PARITY_ODD, UART_PARITY_TYPE_ODD);
  jerryxx_set_property_number(exports, MSTR_UART_PARITY_EVEN, UART_PARITY_TYPE_EVEN);
  jerryxx_set_property_number(exports, MSTR_UART_FLOW_NONE, UART_FLOW_NONE);
  jerryxx_set_property_number(exports, MSTR_UART_FLOW_RTS, UART_FLOW_RTS);
  jerryxx_set_property_number(exports, MSTR_UART_FLOW_CTS, UART_FLOW_CTS);
  jerryxx_set_property_number(exports, MSTR_UART_FLOW_RTS_CTS, UART_FLOW_RTS_CTS);
  jerry_release_value (uart_ctor);

  return exports;
}
