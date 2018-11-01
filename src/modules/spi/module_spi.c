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
#include "spi_magic_strings.h"
#include "spi.h"

#define SPI_DEFAULT_MODE SPI_MODE_0
#define SPI_DEFAULT_BAUDRATE 921600
#define SPI_DEFAULT_BITORDER SPI_BITORDER_MSB
#define SPI_DEFAULT_BITS 8

/**
 * SPI() constructor
 */
JERRYXX_FUN(spi_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");

  // read parameters
  uint8_t bus = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t options = JERRYXX_GET_ARG_OPT(1, NULL);
  uint8_t mode = SPI_DEFAULT_MODE;
  uint32_t baudrate = SPI_DEFAULT_BAUDRATE;
  uint8_t bitorder = SPI_DEFAULT_BITORDER;
  uint8_t bits = SPI_DEFAULT_BITS;
  if (jerry_value_is_object(options)) {
    mode = (uint8_t) jerryxx_get_property_number(options,  MSTR_SPI_MODE, SPI_DEFAULT_MODE);
    baudrate = (uint32_t) jerryxx_get_property_number(options,  MSTR_SPI_BAUDRATE, SPI_DEFAULT_BAUDRATE);
    bitorder = (uint8_t) jerryxx_get_property_number(options,  MSTR_SPI_BITORDER, SPI_DEFAULT_BITORDER);
    bits = (uint8_t) jerryxx_get_property_number(options,  MSTR_SPI_BITS, SPI_DEFAULT_BITS);
  }
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BUS, bus);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_MODE, mode);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BAUDRATE, baudrate);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BITORDER, bitorder);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BITS, bits);

  // initialize the bus
  spi_setup(bus, (spi_mode_t) mode, baudrate, (spi_bitorder_t) bitorder, bits);
  return jerry_create_undefined();
}


/** 
 * SPI.prototype.transfer() function
 */
JERRYXX_FUN(spi_transfer_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // write data to the bus
  if (jerry_value_is_array(data)) { /* for Array<number> */
    size_t len = jerry_get_array_length(data);
    uint8_t tx_buf[len];
    uint8_t rx_buf[len];
    for (int i = 0; i < len; i++) {
      jerry_value_t item = jerry_get_property_by_index(data, i);
      if (jerry_value_is_number(item)) {
        tx_buf[i] = (uint8_t) jerry_get_number_value(item);
      } else {
        tx_buf[i] = 0; // write 0 for non-number item.
      }
    }
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret > -1) {
      jerry_value_t array_buffer = jerry_create_arraybuffer(len);
      jerry_arraybuffer_write(array_buffer, 0, rx_buf, len);
      return array_buffer;
    } else {
      return jerry_create_null();
    }
  } else if (jerry_value_is_arraybuffer(data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(data);
    uint8_t tx_buf[len];
    uint8_t rx_buf[len];
    jerry_arraybuffer_read(data, 0, tx_buf, len);
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret > -1) {
      jerry_value_t array_buffer = jerry_create_arraybuffer(len);
      jerry_arraybuffer_write(array_buffer, 0, rx_buf, len);
      return array_buffer;
    } else {
      return jerry_create_null();
    }
  } else if (jerry_value_is_typedarray(data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t tx_buf[len];
    uint8_t rx_buf[len];
    jerry_arraybuffer_read(array_buffer, 0, tx_buf, len);
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret > -1) {
      jerry_value_t array_buffer = jerry_create_arraybuffer(len);
      jerry_arraybuffer_write(array_buffer, 0, rx_buf, len);
      return array_buffer;
    } else {
      return jerry_create_null();
    }
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(data);
    uint8_t tx_buf[len];
    uint8_t rx_buf[len];
    jerry_string_to_char_buffer(data, tx_buf, len);
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret > -1) {
      jerry_value_t array_buffer = jerry_create_arraybuffer(len);
      jerry_arraybuffer_write(array_buffer, 0, rx_buf, len);
      return array_buffer;
    } else {
      return jerry_create_null();
    }
  } else {
    return JERRYXX_CREATE_ERROR("The data argument must be one of string, Array<number>, ArrayBuffer or TypedArray.");
  }
}


/** 
 * SPI.prototype.send() function
 */
JERRYXX_FUN(spi_send_fn) {
  JERRYXX_CHECK_ARG(0, "data");
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // write data to the bus
  int ret = -1;
  if (jerry_value_is_array(data)) { /* for Array<number> */
    size_t len = jerry_get_array_length(data);
    uint8_t tx_buf[len];
    for (int i = 0; i < len; i++) {
      jerry_value_t item = jerry_get_property_by_index(data, i);
      if (jerry_value_is_number(item)) {
        tx_buf[i] = (uint8_t) jerry_get_number_value(item);
      } else {
        tx_buf[i] = 0; // write 0 for non-number item.
      }
    }
    ret = spi_send(bus, tx_buf, len, timeout);
  } else if (jerry_value_is_arraybuffer(data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(data);
    uint8_t tx_buf[len];
    jerry_arraybuffer_read(data, 0, tx_buf, len);
    ret = spi_send(bus, tx_buf, len, timeout);
  } else if (jerry_value_is_typedarray(data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t tx_buf[len];
    jerry_arraybuffer_read(array_buffer, 0, tx_buf, len);
    ret = spi_send(bus, tx_buf, len, timeout);
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(data);
    uint8_t tx_buf[len];
    jerry_string_to_char_buffer(data, tx_buf, len);
    ret = spi_send(bus, tx_buf, len, timeout);
  } else {
    return JERRYXX_CREATE_ERROR("The data argument must be one of string, Array<number>, ArrayBuffer or TypedArray.");
  }
  return jerry_create_number(ret);
}


/** 
 * SPI.prototype.recv() function
 */
JERRYXX_FUN(spi_recv_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  uint8_t length = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  uint32_t timeout = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // recv data
  uint8_t buf[length];
  int ret = spi_recv(bus, buf, length, timeout);

  // return an array buffer
  if (ret > -1) {
    jerry_value_t array_buffer = jerry_create_arraybuffer(length);
    jerry_arraybuffer_write(array_buffer, 0, buf, length);
    return array_buffer;
  }
  return jerry_create_null();
}

/** 
 * SPI.prototype.close() function
 */
JERRYXX_FUN(spi_close_fn) {
  // check this.bus number
  uint8_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return JERRYXX_CREATE_ERROR("SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);

  // close the bus
  int ret = spi_close(bus);
  if (ret < 0) {
    return JERRYXX_CREATE_ERROR("Failed to close SPI bus.");
  }

  // delete this.bus property
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);

  return jerry_create_undefined();
}

/** 
 * Initialize 'spi' module and return exports
 */
jerry_value_t module_spi_init() {
  /* SPI class */
  jerry_value_t spi_ctor = jerry_create_external_function(spi_ctor_fn);
  jerry_value_t spi_prototype = jerry_create_object();
  jerryxx_set_property(spi_ctor, "prototype", spi_prototype);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_TRANSFER, spi_transfer_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_SEND, spi_send_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_RECV, spi_recv_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_CLOSE, spi_close_fn);
  jerry_release_value (spi_prototype);

  /* spi module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_SPI_SPI, spi_ctor);
  jerry_release_value (spi_ctor);

  return exports;
}
