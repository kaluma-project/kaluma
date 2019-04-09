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


/**
 * I2C() constructor
 */
JERRYXX_FUN(i2c_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");

  i2c_mode_t mode = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, I2C_MASTER);
  //Master mode support only
  if (mode != I2C_MASTER)
    return JERRYXX_CREATE_ERROR("Invalid I2C mode.");
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_MODE, mode);

  // check this.bus number
  uint8_t bus = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_BUS, bus);

  // initialize the bus
  if (mode == I2C_SLAVE) { /* slave mode */
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "address");
    uint8_t address = (uint8_t) JERRYXX_GET_ARG_NUMBER(2);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_I2C_ADDRESS, address);
    int ret = i2c_setup_slave(bus, address);
    if (ret < 0) {
      return JERRYXX_CREATE_ERROR("Failed to initialize I2C bus.");
    }
  } else { /* master mode */
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "speed");
    int32_t speed = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 100000);
    if (speed >=400000)
      speed = 400000; //Max is 400KHz
    int ret = i2c_setup_master(bus, speed);
    if (ret < 0) {
      return JERRYXX_CREATE_ERROR("Failed to initialize I2C bus.");
    }
  }
  return jerry_create_undefined();
}


/**
 * I2C.prototype.write() function
 */
JERRYXX_FUN(i2c_write_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // check the mode (determine slave mode or master mode)
  i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);

  // read optional parameters (address, timeout)
  uint8_t address = 0;
  uint32_t timeout = 5000;
  if (i2cmode == I2C_SLAVE) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    address = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
  }

  // write data to the bus
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
    if (i2cmode == I2C_SLAVE) {
      ret = i2c_write_slave(bus, buf, len, timeout);
    } else {
      ret = i2c_write_master(bus, address, buf, len, timeout);
    }
  } else if (jerry_value_is_arraybuffer(data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(data);
    uint8_t buf[len];
    jerry_arraybuffer_read(data, 0, buf, len);
    if (i2cmode == I2C_SLAVE) {
      ret = i2c_write_slave(bus, buf, len, timeout);
    } else {
      ret = i2c_write_master(bus, address, buf, len, timeout);
    }
  } else if (jerry_value_is_typedarray(data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t buf[len];
    jerry_arraybuffer_read(array_buffer, 0, buf, len);
    if (i2cmode == I2C_SLAVE) {
      ret = i2c_write_slave(bus, buf, len, timeout);
    } else {
      ret = i2c_write_master(bus, address, buf, len, timeout);
    }
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(data);
    uint8_t buf[len];
    jerry_string_to_char_buffer(data, buf, len);
    if (i2cmode == I2C_SLAVE) {
      ret = i2c_write_slave(bus, buf, len, timeout);
    } else {
      ret = i2c_write_master(bus, address, buf, len, timeout);
    }
  }
  return jerry_create_number(ret);
}


/**
 * I2C.prototype.read() function
 */
JERRYXX_FUN(i2c_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  uint8_t length = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // check the mode (determine slave mode or master mode)
  i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);

  // read data with optional parameters (address, timeout)
  uint8_t address = 0;
  uint32_t timeout = 5000;
  uint8_t buf[length];
  int ret = -1;
  if (i2cmode == I2C_SLAVE) {
    JERRYXX_CHECK_ARG_NUMBER_OPT(1, "timeout");
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
    ret = i2c_read_slave(bus, buf, length, timeout);
  } else {
    JERRYXX_CHECK_ARG_NUMBER(1, "address");
    JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
    address = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
    timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 5000);
    ret = i2c_read_master(bus, address, buf, length, timeout);
  }

  // return an array buffer
  if (ret > -1) {
    jerry_value_t array_buffer = jerry_create_arraybuffer(length);
    jerry_arraybuffer_write(array_buffer, 0, buf, length);
    return array_buffer;
  }
  return jerry_create_null();
}

/**
 * I2C.prototype.memWrite() function
 */
JERRYXX_FUN(i2c_memwrite_fn) {
  JERRYXX_CHECK_ARG(0, "memAddr");
  uint16_t memAddress = (uint16_t) JERRYXX_GET_ARG_NUMBER(0);

  JERRYXX_CHECK_ARG(1, "data");
  jerry_value_t data = JERRYXX_GET_ARG(1);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // check the mode (determine slave mode or master mode)
  i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);
  if (i2cmode == I2C_SLAVE)
    return JERRYXX_CREATE_ERROR("This function runs in master mode only.");

  JERRYXX_CHECK_ARG_NUMBER(2, "slaveaddr");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "memAddr16bit");
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "timeout");
  uint8_t address = (uint8_t) JERRYXX_GET_ARG_NUMBER(2);
  uint16_t memAddr16 = (uint16_t) JERRYXX_GET_ARG_NUMBER_OPT(3, 0);
  uint32_t timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(4, 5000);

  // write data to the bus
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
    ret = i2c_memWrite_master(bus, address, memAddress, memAddr16, buf, len, timeout);
  } else if (jerry_value_is_arraybuffer(data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(data);
    uint8_t buf[len];
    jerry_arraybuffer_read(data, 0, buf, len);
    ret = i2c_memWrite_master(bus, address, memAddress, memAddr16, buf, len, timeout);
  } else if (jerry_value_is_typedarray(data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t buf[len];
    jerry_arraybuffer_read(array_buffer, 0, buf, len);
    ret = i2c_memWrite_master(bus, address, memAddress, memAddr16, buf, len, timeout);
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(data);
    uint8_t buf[len];
    jerry_string_to_char_buffer(data, buf, len);
    ret = i2c_memWrite_master(bus, address, memAddress, memAddr16, buf, len, timeout);
  }
  return jerry_create_number(ret);
}


/**
 * I2C.prototype.memRead() function
 */
JERRYXX_FUN(i2c_memread_fn) {
  JERRYXX_CHECK_ARG(0, "memAddr");
  uint16_t memAddress = (uint16_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_CHECK_ARG_NUMBER(1, "length");
  uint8_t length = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
  uint8_t buf[length];

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // check the mode (determine slave mode or master mode)
  i2c_mode_t i2cmode = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_MODE);
  if (i2cmode == I2C_SLAVE)
    return JERRYXX_CREATE_ERROR("This function runs in master mode only.");

  JERRYXX_CHECK_ARG_NUMBER(2, "slaveaddr");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "memAddr16bit");
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "timeout");
  uint8_t address = (uint8_t) JERRYXX_GET_ARG_NUMBER(2);
  uint16_t memAddr16 = (uint16_t) JERRYXX_GET_ARG_NUMBER_OPT(3, 0);
  uint32_t timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(4, 5000);

  int ret = i2c_memRead_master(bus, address, memAddress, memAddr16, buf, length, timeout);

  // return an array buffer
  if (ret > -1) {
    jerry_value_t array_buffer = jerry_create_arraybuffer(length);
    jerry_arraybuffer_write(array_buffer, 0, buf, length);
    return array_buffer;
  }
  return jerry_create_null();
}

/**
 * I2C.prototype.close() function
 */
JERRYXX_FUN(i2c_close_fn) {
  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // close the bus
  int ret = i2c_close(bus);
  if (ret < 0) {
    return JERRYXX_CREATE_ERROR("Failed to close I2C bus.");
  }

  // delete this.bus property
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_I2C_BUS);

  return jerry_create_undefined();
}


/**
 * Initialize 'i2c' module
 */
jerry_value_t module_i2c_init() {
  /* I2C class */
  jerry_value_t i2c_ctor = jerry_create_external_function(i2c_ctor_fn);
  jerry_value_t i2c_prototype = jerry_create_object();
  jerryxx_set_property(i2c_ctor, "prototype", i2c_prototype);
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_MASTERMODE, I2C_MASTER);
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_SLAVEMODE, I2C_SLAVE);
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_STDSPEED, 100000); //100kbps
  jerryxx_set_property_number(i2c_ctor, MSTR_I2C_FULLSPEED, 400000); //400kbps
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_WRITE, i2c_write_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_READ, i2c_read_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_MEM_WRITE, i2c_memwrite_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_MEM_READ, i2c_memread_fn);
  jerryxx_set_property_function(i2c_prototype, MSTR_I2C_CLOSE, i2c_close_fn);
  jerry_release_value (i2c_prototype);

  /* i2c module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_I2C_I2C, i2c_ctor);
  jerry_release_value (i2c_ctor);

  return exports;
}
