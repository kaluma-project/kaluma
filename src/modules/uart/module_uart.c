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
#define UART_DEFAULT_PARITY 0
#define UART_DEFAULT_STOP 1
#define UART_DEFAULT_FLOW 0
#define UART_DEFAULT_BUFFERSIZE 1024

/**
 * UART() constructor
 */
JERRYXX_FUN(uart_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "port");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");

  // read parameters
  uint8_t port = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t options = JERRYXX_GET_ARG_OPT(1, NULL);
  uint32_t baudrate = UART_DEFAULT_BAUDRATE;
  uint32_t bits = UART_DEFAULT_BITS;
  uint32_t parity = UART_DEFAULT_PARITY;
  uint32_t stop = UART_DEFAULT_STOP;
  uint32_t flow = UART_DEFAULT_FLOW;
  uint32_t buffer_size = UART_DEFAULT_BUFFERSIZE;
  if (jerry_value_is_object(options)) {
    baudrate = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BAUDRATE, UART_DEFAULT_BAUDRATE);
    bits = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BITS, UART_DEFAULT_BITS);
    parity = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_PARITY, UART_DEFAULT_PARITY);
    stop = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_STOP, UART_DEFAULT_STOP);
    flow = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_FLOW, UART_DEFAULT_FLOW);
    buffer_size = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BUFFERSIZE, UART_DEFAULT_BUFFERSIZE);
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_PORT, port);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BAUDRATE, baudrate);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BITS, bits);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_PARITY, parity);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_STOP, stop);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_FLOW, flow);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BUFFERSIZE, buffer_size);
  if (!is_uart_port(port)) {
    return JERRYXX_CREATE_ERROR("Not supported UART port");
  }

  // initialize the port
  uart_setup(port, baudrate, bits, parity, stop, flow, buffer_size);
  return jerry_create_undefined();
}

JERRYXX_FUN(uart_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);

  // check this.port
  uint8_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return JERRYXX_CREATE_ERROR("UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);

  // write data to the port
  int ret = -1;
  if (jerry_value_is_array(data)) { /* for Array<number> */
    size_t len = jerry_get_array_length(data);
    uint8_t buf[len];
    for (int i = 0; i < len; i++) {
      jerry_value_t item = jerry_get_property_by_index(data, i);
      if (jerry_value_is_number(item)) {
        buf[i] = (uint8_t) jerry_get_number_value(item);
      } else {
        buf[i] = 0; // write 0 for non-number item.
      }
    }
    ret = uart_write(port, buf, len);
  } else if (jerry_value_is_arraybuffer(data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(data);
    uint8_t buf[len];
    jerry_arraybuffer_read(data, 0, buf, len);
    ret = uart_write(port, buf, len);
  } else if (jerry_value_is_typedarray(data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t buf[len];
    jerry_arraybuffer_read(array_buffer, 0, buf, len);
    ret = uart_write(port, buf, len);
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(data);
    uint8_t buf[len];
    jerry_string_to_char_buffer(data, buf, len);
    ret = uart_write(port, buf, len);
  } else {
    return JERRYXX_CREATE_ERROR("The data argument must be one of string, Array<number>, ArrayBuffer or TypedArray.");
  }
  return jerry_create_undefined();
}


/** 
 * UART.prototype.available() function
 */
JERRYXX_FUN(uart_available_fn) {
  // check this.port
  uint8_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return JERRYXX_CREATE_ERROR("UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);

  // check available data in read buffer
  uint32_t len = uart_available(port);
  return jerry_create_number(len);
}


/** 
 * UART.prototype.read() function
 */
JERRYXX_FUN(uart_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "length");
  uint32_t length = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(0, 1);

  // check this.port
  uint8_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return JERRYXX_CREATE_ERROR("UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);

  uint8_t buf[length];
  int ret = uart_read(port, buf, length);
  if (ret > -1) {
    jerry_value_t array_buffer = jerry_create_arraybuffer(length);
    jerry_arraybuffer_write(array_buffer, 0, buf, length);
    return array_buffer;
  }
  return jerry_create_null();
}


/** 
 * UART.prototype.listen() function
 */
JERRYXX_FUN(uart_listen_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint8_t trigger_type = 1; // 0 = buffer_size, 1 = end_char
  uint32_t trigger_buffer_size = 1;
  uint8_t trigger_end_char = '\n';
  if (JERRYXX_GET_ARG_COUNT > 1) {
    jerry_value_t trigger = JERRYXX_GET_ARG(1);
    if (jerry_value_is_number(trigger)) {
      trigger_buffer_size = (uint32_t) jerry_get_number_value(trigger);
      trigger_type = 0;
    } else if (jerry_value_is_string(trigger)) {
      JERRYXX_GET_ARG_STRING_AS_CHAR(1, trigger_string);
      if (trigger_string_sz > 0) {
        trigger_end_char = trigger_string[0];
      }
    }
  }

  // check this.port
  uint8_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return JERRYXX_CREATE_ERROR("UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);

  // check this._handle_id
  // if exists, stop and close the handle.

  io_poll_handle_t *poll = malloc(sizeof(io_poll_handle_t));
  io_poll_init(poll);
  // watch->watch_js_cb = callback;
  io_poll_read_start(poll);

  // set this._handle_id = poll->base.id;

  return jerry_create_undefined();
}


/** 
 * UART.prototype.close() function
 */
JERRYXX_FUN(uart_close_fn) {
  // check this.port
  uint8_t port_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_PORT);
  if (!jerry_value_is_number(port_value)) {
    return JERRYXX_CREATE_ERROR("UART port is not initialized.");
  }
  uint8_t port = (uint8_t) jerry_get_number_value(port_value);

  // close the port
  int ret = uart_close(port);
  if (ret < 0) {
    return JERRYXX_CREATE_ERROR("Failed to close UART port.");
  }

  // delete this.port
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_UART_PORT);

  // TODO: check this._handle_id
  // if exists, stop and close the handle.

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
  jerryxx_set_property_function(uart_prototype, MSTR_UART_AVAILABLE, uart_available_fn);
  jerryxx_set_property_function(uart_prototype, MSTR_UART_READ, uart_read_fn);
  jerryxx_set_property_function(uart_prototype, MSTR_UART_LISTEN, uart_listen_fn);
  jerryxx_set_property_function(uart_prototype, MSTR_UART_CLOSE, uart_close_fn);
  jerry_release_value (uart_prototype);

  /* uart module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_UART_UART, uart_ctor);
  jerry_release_value (uart_ctor);

  return exports;
}
