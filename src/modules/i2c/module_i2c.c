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
#include "i2c_magic_strings.h"
#include "i2c.h"

JERRYXX_FUN(i2c_ctor_fn) {
  return jerry_create_undefined();
}

JERRYXX_FUN(i2c_setup_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "address");
  uint8_t bus = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_BUS, bus);
  if (!is_i2c_bus(bus)) {
    return JERRXX_CREATE_ERROR("Not supported I2C bus");
  }
  if (JERRYXX_GET_ARG_COUNT > 1) { /* slave mode */
    uint8_t address = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_ADDRESS, address);
    int ret = i2c_setup_slave(bus, address);
    // TODO: Handle result state code (ret)
  } else { /* master mode */
    int ret = i2c_setup_master(bus);
    // TODO: Handle result state code (ret)
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(i2c_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);
  // Get bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRXX_CREATE_ERROR("I2C bus is not setup.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  uint8_t address = 0;
  uint32_t timeout = 5000;
  uint8_t address_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_ADDRESS);
  bool is_slave_mode = jerry_value_is_number(address_value);
  if (is_slave_mode) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    address = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
  }
  if (jerry_value_is_array(data)) {
    uint32_t len = jerry_get_array_length(data);
    uint8_t buf[len];
    for (int i = 0; i < len; i++) {
      jerry_value_t item = jerry_get_property_by_index(data, i);
      if (jerry_value_is_number(item)) {
        buf[i] = (uint8_t) jerry_get_number_value(item);
      } else {
        // TODO: data array has non-number object element
      }
    }
    if (is_slave_mode) {
      i2c_write_slave(bus, buf, len, timeout);
    } else {
      i2c_write_master(bus, address, buf, len, timeout);
    }
  } else {
    // TODO: support string, arraybuffer
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(i2c_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  uint8_t length = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  // Get bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRXX_CREATE_ERROR("I2C bus is not setup.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  uint8_t buf[length];
  uint8_t address = 0;
  uint32_t timeout = 5000;
  uint8_t address_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_ADDRESS);
  bool is_slave_mode = jerry_value_is_number(address_value);
  if (is_slave_mode) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
    i2c_read_slave(bus, buf, length, timeout);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    address = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
    i2c_read_master(bus, address, buf, length, timeout);
  }
  // TODO: return ArrayBuffer of buf array.
  return jerry_create_undefined();
}

JERRYXX_FUN(i2c_close_fn) {
  // Get bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRXX_CREATE_ERROR("I2C bus is not setup.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  i2c_close(bus); //TODO: Handle result code
  return jerry_create_undefined();
}

jerry_value_t module_i2c_init() {
  /* I2C constructor */
  jerry_value_t ctor = jerry_create_external_function(i2c_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property_object(ctor, "prototype", prototype);
  jerry_release_value (prototype);
  /* I2C instance properties */
  jerryxx_set_property_function(prototype, MSTR_I2C_SETUP, i2c_setup_fn);
  jerryxx_set_property_function(prototype, MSTR_I2C_WRITE, i2c_write_fn);
  jerryxx_set_property_function(prototype, MSTR_I2C_READ, i2c_read_fn);
  return ctor;
}
