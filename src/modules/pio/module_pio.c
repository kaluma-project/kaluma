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

#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pio.h"
#include "pio_magic_strings.h"

/**
 * pio_native constructor
 * args:
 *   port {number}
 *   options {Object}
 *   callback (function(data))
 */
JERRYXX_FUN(pio_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "port");
  JERRYXX_CHECK_ARG(1, "code");

  // read parameters
  uint8_t port = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t code = JERRYXX_GET_ARG(1);
  int code_len;
  uint16_t *code_arr;

  if (jerry_value_is_typedarray(code) &&
      jerry_get_typedarray_type(code) ==
          JERRY_TYPEDARRAY_UINT16) { /* Uint16Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_get_typedarray_buffer(code, &byteOffset, &byteLength);
    code_len = jerry_get_arraybuffer_byte_length(array_buffer);
    code_len /= 2;  // because it's 16bit array
    code_arr = (uint16_t *)jerry_get_arraybuffer_pointer(array_buffer);
    jerry_release_value(array_buffer);
  } else {
    return jerry_create_error(
        JERRY_ERROR_TYPE,
        (const jerry_char_t
             *)"The code argument must be Uint16Array or string.");
  }
  if (km_pio_port_init(port, code_arr, code_len) < 0) {
    return jerry_create_error(JERRY_ERROR_COMMON,
                              (const jerry_char_t *)"Port init fails.");
  }

  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PIO_PORT, port);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_PIO_CODE, code);
  return jerry_create_undefined();
}

/**
 * PIO.prototype.smSetup() function
 */
JERRYXX_FUN(pio_sm_setup_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "sm");
  JERRYXX_CHECK_ARG_OBJECT(1, "options");

  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  if (sm >= KM_PIO_NO_SM) {
    return jerry_create_error(
        JERRY_ERROR_TYPE,
        (const jerry_char_t *)"State machine number is not supported.");
  }
  // check this.port
  jerry_value_t port_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PIO_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(
        JERRY_ERROR_TYPE, (const jerry_char_t *)"PIO port is not a number.");
  }
  uint8_t port = (uint8_t)jerry_get_number_value(port_value);
  jerry_release_value(port_value);
  jerry_value_t options = JERRYXX_GET_ARG(1);
  uint8_t pin_out =
      (uint8_t)jerryxx_get_property_number(options, MSTR_PIO_PIN_OUT, 0xFF);
  uint8_t pin_in =
      (uint8_t)jerryxx_get_property_number(options, MSTR_PIO_PIN_IN, 0xFF);
  uint8_t pin_out_cnt =
      (uint8_t)jerryxx_get_property_number(options, MSTR_PIO_PIN_OUT_CNT, 1);
  uint8_t pin_in_cnt =
      (uint8_t)jerryxx_get_property_number(options, MSTR_PIO_PIN_IN_CNT, 1);
  int ret = km_pio_sm_setup(port, sm);
  if ((ret == 0) && (pin_out != 0xFF)) {
    ret = km_pio_sm_set_out(port, sm, pin_out, pin_out_cnt);
  }
  if ((ret == 0) && (pin_in != 0xFF)) {
    ret = km_pio_sm_set_in(port, sm, pin_in, pin_in_cnt);
  }
  if (ret == 0) {
    ret = km_pio_sm_init(port, sm);
  }
  if (ret < 0) {
    return jerry_create_error(
        JERRY_ERROR_COMMON, (const jerry_char_t *)"State machine init fails.");
  }
  return jerry_create_number(sm);
}

/**
 * PIO.prototype.smEnable() function
 */
JERRYXX_FUN(pio_sm_enable_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "sm");
  JERRYXX_CHECK_ARG_BOOLEAN_OPT(1, "enable");

  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  if (sm >= KM_PIO_NO_SM) {
    return jerry_create_error(
        JERRY_ERROR_TYPE,
        (const jerry_char_t *)"State machine number is not supported.");
  }
  // check this.port
  jerry_value_t port_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PIO_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(
        JERRY_ERROR_TYPE, (const jerry_char_t *)"PIO port is not a number.");
  }
  uint8_t port = (uint8_t)jerry_get_number_value(port_value);
  jerry_release_value(port_value);
  uint8_t en = (uint8_t)JERRYXX_GET_ARG_BOOLEAN_OPT(1, true);

  if (km_pio_sm_enable(port, sm, en) < 0) {
    return jerry_create_error(
        JERRY_ERROR_COMMON,
        (const jerry_char_t *)"Enabling state machine fails.");
  }
  return jerry_create_number(sm);
}

/**
 * PIO.prototype.put() function
 */
JERRYXX_FUN(pio_put_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "sm");
  JERRYXX_CHECK_ARG_NUMBER(1, "data");
  // read parameters
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint32_t data = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  // check this.port
  jerry_value_t port_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PIO_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(
        JERRY_ERROR_TYPE, (const jerry_char_t *)"PIO port is not a number.");
  }
  uint8_t port = (uint8_t)jerry_get_number_value(port_value);
  if (km_pio_put_fifo(port, sm, data) < 0) {
    return jerry_create_error(JERRY_ERROR_COMMON,
                              (const jerry_char_t *)"Put FIFO fails.");
  }
  return jerry_create_undefined();
}

/**
 * PIO.prototype.get() function
 */
JERRYXX_FUN(pio_get_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "sm");
  // read parameters
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  // check this.port
  jerry_value_t port_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PIO_PORT);
  if (!jerry_value_is_number(port_value)) {
    return jerry_create_error(
        JERRY_ERROR_TYPE, (const jerry_char_t *)"PIO port is not a number.");
  }
  uint8_t port = (uint8_t)jerry_get_number_value(port_value);
  int8_t err;
  uint32_t data = km_pio_get_fifo(port, sm, &err);
  if (err) {
    return jerry_create_error(JERRY_ERROR_COMMON,
                              (const jerry_char_t *)"Get FIFO fails.");
  }
  return jerry_create_number(data);
}

/**
 * PIO.prototype.close() function
 */
JERRYXX_FUN(pio_close_fn) {
  uint8_t port = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  km_pio_close(port);
  return jerry_create_undefined();
}

/**
 * Initialize 'pio' module and return exports
 */
jerry_value_t module_pio_init() {
  /* PIO class */
  jerry_value_t pio_ctor = jerry_create_external_function(pio_ctor_fn);
  jerry_value_t pio_prototype = jerry_create_object();
  jerryxx_set_property(pio_ctor, "prototype", pio_prototype);
  jerryxx_set_property_function(pio_prototype, MSTR_PIO_SM_SETUP,
                                pio_sm_setup_fn);
  jerryxx_set_property_function(pio_prototype, MSTR_PIO_SM_ENABLE,
                                pio_sm_enable_fn);
  jerryxx_set_property_function(pio_prototype, MSTR_PIO_PUT, pio_put_fn);
  jerryxx_set_property_function(pio_prototype, MSTR_PIO_GET, pio_get_fn);
  jerryxx_set_property_function(pio_prototype, MSTR_PIO_CLOSE, pio_close_fn);
  jerry_release_value(pio_prototype);

  /* pio module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_PIO_PIO, pio_ctor);
  jerry_release_value(pio_ctor);

  return exports;
}
