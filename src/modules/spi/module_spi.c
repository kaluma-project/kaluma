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
#define SPI_DEFAULT_BAUDRATE 3000000
#define SPI_DEFAULT_BITORDER SPI_BITORDER_MSB

static void buffer_free_cb(void *native_p) {
  free(native_p);
}

/**
 * SPI() constructor
 */
JERRYXX_FUN(spi_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");

  // read parameters
  uint8_t bus = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  uint8_t mode = SPI_DEFAULT_MODE;
  uint32_t baudrate = SPI_DEFAULT_BAUDRATE;
  uint8_t bitorder = SPI_DEFAULT_BITORDER;

  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t options = JERRYXX_GET_ARG(1);
    if (jerry_value_is_object(options)) {
      mode = (uint8_t) jerryxx_get_property_number(options, MSTR_SPI_MODE, SPI_DEFAULT_MODE);
      baudrate = (uint32_t) jerryxx_get_property_number(options, MSTR_SPI_BAUDRATE, SPI_DEFAULT_BAUDRATE);
      bitorder = (uint8_t) jerryxx_get_property_number(options, MSTR_SPI_BITORDER, SPI_DEFAULT_BITORDER);
    }
  }

  if (bitorder != SPI_BITORDER_LSB)
    bitorder = SPI_BITORDER_MSB;
  if (mode < 0 || mode > 3)
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) "SPI mode error.");
  // initialize the bus
  if (spi_setup(bus, (spi_mode_t) mode, baudrate, (spi_bitorder_t) bitorder) == SPIPORT_ERROR) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "SPI port setup fail.");
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BUS, bus);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_MODE, mode);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BAUDRATE, baudrate);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BITORDER, bitorder);
    return jerry_create_undefined();
  }
}


/**
 * SPI.prototype.transfer() function
 */
JERRYXX_FUN(spi_transfer_fn) {
  JERRYXX_CHECK_ARG(0, "data");

  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  jerry_value_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  jerry_release_value(bus_value);

  // write data to the bus
  if (jerry_value_is_typedarray(data) &&
      jerry_get_typedarray_type(data) == JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t *tx_buf = jerry_get_arraybuffer_pointer(array_buffer);
    uint8_t *rx_buf = malloc(len);
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret == SPIPORT_ERROR) {
      free(rx_buf);
      return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to transfer data via SPI bus.");
    } else {
      jerry_value_t buffer = jerry_create_arraybuffer_external(len, rx_buf,
        buffer_free_cb);
      jerry_value_t array = jerry_create_typedarray_for_arraybuffer(
        JERRY_TYPEDARRAY_UINT8, buffer);
      jerry_release_value(buffer);
      return array;
    }
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t tx_buf[len];
    uint8_t *rx_buf = malloc(len);
    jerryxx_string_to_ascii_char_buffer(data, tx_buf, len);
    int ret = spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret == SPIPORT_ERROR) {
      free(rx_buf);
      return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to transfer data via SPI bus.");
    } else {
      jerry_value_t buffer = jerry_create_arraybuffer_external(len, rx_buf,
        buffer_free_cb);
      jerry_value_t array = jerry_create_typedarray_for_arraybuffer(
        JERRY_TYPEDARRAY_UINT8, buffer);
      jerry_release_value(buffer);
      return array;
    }
  } else {
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "The data argument must be Uint8Array or string.");
  }
}


/**
 * SPI.prototype.send() function
 */
JERRYXX_FUN(spi_send_fn) {
  JERRYXX_CHECK_ARG(0, "data");

  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
  uint32_t count = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 1);

  // check this.bus number
  jerry_value_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  jerry_release_value(bus_value);

  // write data to the bus
  int ret = SPIPORT_ERROR;
  if (jerry_value_is_typedarray(data) &&
      jerry_get_typedarray_type(data) == JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t *tx_buf = jerry_get_arraybuffer_pointer(array_buffer);
    for (int c = 0; c < count; c++) {
      ret = spi_send(bus, tx_buf, len, timeout);
      if (ret < 0) break;
    }
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t tx_buf[len];
    jerryxx_string_to_ascii_char_buffer(data, tx_buf, len);
    for (int c = 0; c < count; c++) {
      ret = spi_send(bus, tx_buf, len, timeout);
      if (ret < 0) break;
    }
  } else {
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "The data argument must be Uint8Array or string.");
  }
  if (ret == SPIPORT_ERROR)
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to send data via SPI bus.");
  else
    return jerry_create_number(ret);
}


/**
 * SPI.prototype.recv() function
 */
JERRYXX_FUN(spi_recv_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  uint32_t length = (uint32_t) JERRYXX_GET_ARG_NUMBER(0);
  uint32_t timeout = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  jerry_value_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "I2C bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  jerry_release_value(bus_value);

  // recv data
  uint8_t *buf = malloc(length);
  int ret = spi_recv(bus, buf, length, timeout);

  // return an Uin8Array
  if (ret == SPIPORT_ERROR) {
    free(buf);
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to receive data via SPI bus.");
  } else {
    jerry_value_t array_buffer = jerry_create_arraybuffer_external(length, buf,
        buffer_free_cb);
    jerry_value_t array = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, array_buffer);
    jerry_release_value(array_buffer);
    return array;
  }
}

/**
 * SPI.prototype.close() function
 */
JERRYXX_FUN(spi_close_fn) {
  // check this.bus number
  jerry_value_t bus_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t) jerry_get_number_value(bus_value);
  jerry_release_value(bus_value);

  // close the bus
  int ret = spi_close(bus);
  if (ret == SPIPORT_ERROR) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to close SPI bus.");
  }

  // delete this.bus property
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);

  return jerry_create_undefined();
}

/**
 * Initialize 'spi' module
 */
jerry_value_t module_spi_init() {
  /* SPI class */
  jerry_value_t spi_ctor = jerry_create_external_function(spi_ctor_fn);
  jerry_value_t spi_prototype = jerry_create_object();
  jerryxx_set_property(spi_ctor, "prototype", spi_prototype);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE0, SPI_MODE_0);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE1, SPI_MODE_1);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE2, SPI_MODE_2);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE3, SPI_MODE_3);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MSB, SPI_BITORDER_MSB);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_LSB, SPI_BITORDER_LSB);
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
