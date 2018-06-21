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

#define UART_DEFAULT_BAUDRATE 9600
#define UART_DEFAULT_BITS 8
#define UART_DEFAULT_PARITY 0 // TODO: should be -1
#define UART_DEFAULT_STOP 1
#define UART_DEFAULT_FLOW 0
#define UART_DEFAULT_BUFFERSIZE 1024

JERRYXX_FUN(uart_ctor_fn) {
  return jerry_create_undefined();
}

JERRYXX_FUN(uart_setup_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");
  uint8_t bus = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t options = JERRYXX_GET_ARG_OPT(1, NULL);
  uint32_t baudrate = UART_DEFAULT_BAUDRATE;
  uint32_t bits = UART_DEFAULT_BITS;
  uint32_t parity = UART_DEFAULT_PARITY; // TODO: Type should be int32_t (allows -1)
  uint32_t stop = UART_DEFAULT_STOP;
  uint32_t flow = UART_DEFAULT_FLOW;
  uint32_t buffer_size = UART_DEFAULT_BUFFERSIZE;
  // TODO: Handle 'dataEvent' property
  if (jerry_value_is_object(options)) {
    baudrate = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BAUDRATE, UART_DEFAULT_BAUDRATE);
    bits = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BITS, UART_DEFAULT_BITS);
    parity = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_PARITY, UART_DEFAULT_PARITY);
    stop = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_STOP, UART_DEFAULT_STOP);
    flow = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_FLOW, UART_DEFAULT_FLOW);
    buffer_size = (uint32_t) jerryxx_get_property_number(options,  MSTR_UART_BUFFERSIZE, UART_DEFAULT_BUFFERSIZE);
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BUS, bus);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BAUDRATE, baudrate);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BITS, bits);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_PARITY, parity);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_STOP, stop);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_FLOW, flow);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_UART_BUFFERSIZE, buffer_size);
  if (!is_uart_bus(bus)) {
    return JERRXX_CREATE_ERROR("Not supported UART bus");
  }
  uart_setup(bus, baudrate, bits, parity, stop, flow, buffer_size);
  return jerry_create_undefined();
}

JERRYXX_FUN(uart_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_UART_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRXX_CREATE_ERROR("UART bus is not setup.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  if (jerry_value_is_string(data)) {
    jerry_size_t data_sz = jerry_get_string_size(data);
    jerry_char_t data_buf[data_sz];
    jerry_string_to_char_buffer(data, data_buf, data_sz);
    uart_write(bus, data_buf, data_sz);
  }
  // TODO: impl for ArrayBuffer and Array<number>.
  return jerry_create_undefined();
}

jerry_value_t module_uart_init() {
  /* UART constructor */
  jerry_value_t ctor = jerry_create_external_function(uart_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property_object(ctor, "prototype", prototype);
  jerry_release_value (prototype);
  /* UART instance properties */
  jerryxx_set_property_function(prototype, MSTR_UART_SETUP, uart_setup_fn);
  jerryxx_set_property_function(prototype, MSTR_UART_WRITE, uart_write_fn);
  return ctor;
}
