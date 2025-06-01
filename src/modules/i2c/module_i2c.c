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
#include "i2c.h"
#include "i2c_magic_strings.h"
#include "jerryscript.h"
#include "jerryxx.h"

#define I2C_DEFAULT_MODE KM_I2C_MASTER
#define I2C_DEFAULT_BAUDRATE 100000  // 100kbps

/**
 * I2C() constructor
 */
JERRYXX_FUN(i2c_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");
  uint8_t bus = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);

  km_i2c_pins_t def_pins = km_i2c_get_default_pins(bus);
  km_i2c_pins_t pins = {
      .sda = def_pins.sda,
      .scl = def_pins.scl,
  };
  km_i2c_mode_t mode = I2C_DEFAULT_MODE;
  uint32_t baudrate = I2C_DEFAULT_BAUDRATE;
  uint8_t address = 0;

  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t options = JERRYXX_GET_ARG(1);
    mode =
        jerryxx_get_property_number(options, MSTR_I2C_MODE, I2C_DEFAULT_MODE);
    baudrate = (uint32_t)jerryxx_get_property_number(options, MSTR_I2C_BAUDRATE,
                                                     I2C_DEFAULT_BAUDRATE);
    address =
        (uint32_t)jerryxx_get_property_number(options, MSTR_I2C_ADDRESS, 0);
    pins.sda = (int8_t)jerryxx_get_property_number(options, MSTR_I2C_SDA,
                                                   def_pins.sda);
    pins.scl = (int8_t)jerryxx_get_property_number(options, MSTR_I2C_SCL,
                                                   def_pins.scl);
  }

  // master mode support only
  if (mode != KM_I2C_MASTER)
    return jerry_error_sz(JERRY_ERROR_RANGE, "Unsupported I2C mode.");
  // initialize the bus
  if (mode == KM_I2C_SLAVE) { /* slave mode */
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_ADDRESS, address);
    int ret = km_i2c_setup_slave(bus, address, pins);
    if (ret < 0) {
      return jerry_exception_value(create_system_error(ret), true);
    }
  } else { /* master mode */
    int ret = km_i2c_setup_master(bus, baudrate, pins);
    if (ret < 0) {
      return jerry_exception_value(create_system_error(ret), true);
    }
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_MODE, mode);
  // check this.bus number
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_BUS, bus);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_MODE, mode);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_BAUDRATE, baudrate);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_SDA, pins.sda);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_SCL, pins.scl);
  return jerry_undefined();
}

/**
 * I2C.prototype.write() function
 */
JERRYXX_FUN(i2c_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);
  // check the mode (determine slave mode or master mode)
  km_i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);

  // read optional parameters (address, timeout)
  uint8_t address = 0;
  uint32_t timeout = 5000;
  uint32_t count = 1;
  if (i2cmode == KM_I2C_SLAVE) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t)JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    JERRYXX_CHECK_ARG_NUMBER_OPT(3, "count");
    address = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
    count = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(3, 1);
  }

  // write data to the bus
  int ret = 0;
  if (jerry_value_is_typedarray(data) &&
      jerry_typedarray_type(data) ==
          JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_arraybuffer_size(array_buffer);
    uint8_t *buf = jerry_arraybuffer_data(array_buffer);
    if (i2cmode == KM_I2C_SLAVE) {
      for (int c = 0; c < count; c++) {
        ret = km_i2c_write_slave(bus, buf, len, timeout);
        if (ret < 0) break;
      }
    } else {
      for (int c = 0; c < count; c++) {
        ret = km_i2c_write_master(bus, address, buf, len, timeout);
        if (ret < 0) break;
      }
    }
    jerry_value_free(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t buf[len];
    jerryxx_string_to_ascii_char_buffer(data, buf, len);
    if (i2cmode == KM_I2C_SLAVE) {
      for (int c = 0; c < count; c++) {
        ret = km_i2c_write_slave(bus, buf, len, timeout);
        if (ret < 0) break;
      }
    } else {
      for (int c = 0; c < count; c++) {
        ret = km_i2c_write_master(bus, address, buf, len, timeout);
        if (ret < 0) break;
      }
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The data argument must be Uint8Array or string.");
  }
  if (ret < 0)
    return jerry_exception_value(create_system_error(ret), true);
  else
    return jerry_number(ret);
}

/**
 * I2C.prototype.read() function
 */
JERRYXX_FUN(i2c_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  jerry_length_t length = (jerry_length_t)JERRYXX_GET_ARG_NUMBER(0);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);
  // check the mode (determine slave mode or master mode)
  km_i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);

  // read data with optional parameters (address, timeout)
  uint8_t address = 0;
  uint32_t timeout = 5000;
  uint8_t *buf = malloc(length);
  int ret = 0;
  if (i2cmode == KM_I2C_SLAVE) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t)JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
    ret = km_i2c_read_slave(bus, buf, length, timeout);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    address = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint8_t)JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
    ret = km_i2c_read_master(bus, address, buf, length, timeout);
  }

  // return an Uint8Array
  if (ret < 0) {
    free(buf);
    return jerry_exception_value(create_system_error(ret), true);
  } else {
    jerry_value_t array_buffer =
        jerry_arraybuffer_external(buf, length, NULL);
    jerry_value_t array = jerry_typedarray_with_buffer(
        JERRY_TYPEDARRAY_UINT8, array_buffer);
    jerry_value_free(array_buffer);
    return array;
  }
}

/**
 * I2C.prototype.memWrite() function
 */
JERRYXX_FUN(i2c_memwrite_fn) {
  // check and get args
  JERRYXX_CHECK_ARG(0, "data");
  JERRYXX_CHECK_ARG_NUMBER(1, "address");
  JERRYXX_CHECK_ARG_NUMBER(2, "memAddress");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "memAddressSize");
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "timeout");
  JERRYXX_CHECK_ARG_NUMBER_OPT(5, "count");
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint8_t address = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t memAddress = (uint16_t)JERRYXX_GET_ARG_NUMBER(2);
  uint16_t memAddressSize = (uint16_t)JERRYXX_GET_ARG_NUMBER_OPT(3, 8);
  uint32_t timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(4, 5000);
  uint32_t count = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(5, 1);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);
  // check the mode (determine slave mode or master mode)
  km_i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);
  if (i2cmode == KM_I2C_SLAVE)
    return jerry_error_sz(
        JERRY_ERROR_RANGE,
        "This function runs in master mode only.");

  // write data to the bus
  int ret = 0;
  if (jerry_value_is_typedarray(data) &&
      jerry_typedarray_type(data) ==
          JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_arraybuffer_size(array_buffer);
    uint8_t *buf = jerry_arraybuffer_data(array_buffer);
    for (int c = 0; c < count; c++) {
      ret = km_i2c_mem_write_master(bus, address, memAddress, memAddressSize,
                                    buf, len, timeout);
      if (ret < 0) break;
    }
    jerry_value_free(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t buf[len];
    jerryxx_string_to_ascii_char_buffer(data, buf, len);
    for (int c = 0; c < count; c++) {
      ret = km_i2c_mem_write_master(bus, address, memAddress, memAddressSize,
                                    buf, len, timeout);
      if (ret < 0) break;
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The data argument must be Uint8Array or string.");
  }
  if (ret < 0)
    return jerry_exception_value(create_system_error(ret), true);
  else
    return jerry_number(ret);
}

/**
 * I2C.prototype.memRead() function
 */
JERRYXX_FUN(i2c_memread_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  JERRYXX_CHECK_ARG_NUMBER(1, "address");
  JERRYXX_CHECK_ARG_NUMBER(2, "memAddress");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "memAddressSize");
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "timeout");
  jerry_length_t length = (jerry_length_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t address = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t memAddress = (uint16_t)JERRYXX_GET_ARG_NUMBER(2);
  uint16_t memAddressSize = (uint16_t)JERRYXX_GET_ARG_NUMBER_OPT(3, 8);
  uint32_t timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(4, 5000);

  uint8_t *buf = malloc(length);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);
  // check the mode (determine slave mode or master mode)
  km_i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);
  if (i2cmode == KM_I2C_SLAVE)
    return jerry_error_sz(
        JERRY_ERROR_RANGE,
        "This function runs in master mode only.");

  int ret = km_i2c_mem_read_master(bus, address, memAddress, memAddressSize,
                                   buf, length, timeout);

  // return an Uint8Array
  if (ret < 0) {
    free(buf);
    return jerry_exception_value(create_system_error(ret), true);
  } else {
    jerry_value_t array_buffer =
        jerry_arraybuffer_external(buf, length, NULL);
    jerry_value_t array = jerry_typedarray_with_buffer(
        JERRY_TYPEDARRAY_UINT8, array_buffer);
    jerry_value_free(array_buffer);
    return array;
  }
}

/**
 * I2C.prototype.close() function
 */
JERRYXX_FUN(i2c_close_fn) {
  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);
  // close the bus
  int ret = km_i2c_close(bus);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // delete this.bus property
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);

  return jerry_undefined();
}

/**
 * Initialize 'i2c' module
 */
jerry_value_t module_i2c_init() {
  /* I2C class */
  jerry_value_t i2c_ctor = jerry_function_external(i2c_ctor_fn);
  jerry_value_t i2c_prototype = jerry_object();
  jerryxx_set_property(i2c_ctor, "prototype", i2c_prototype);
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_MASTERMODE, KM_I2C_MASTER);
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_SLAVEMODE, KM_I2C_SLAVE);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_WRITE, i2c_write_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_READ, i2c_read_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_MEM_WRITE,
                                i2c_memwrite_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_MEM_READ,
                                i2c_memread_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_CLOSE, i2c_close_fn);
  jerry_value_free(i2c_prototype);

  /* i2c module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_I2C_I2C, i2c_ctor);
  jerry_value_free(i2c_ctor);

  return exports;
}
