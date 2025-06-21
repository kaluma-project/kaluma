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
#include "jerryscript.h"
#include "jerryxx.h"
#include "spi.h"
#include "spi_magic_strings.h"

#define SPI_DEFAULT_MODE KM_SPI_MODE_0
#define SPI_DEFAULT_BAUDRATE 3000000
#define SPI_DEFAULT_BITORDER KM_SPI_BITORDER_MSB
#define SPI_DEFAULT_PULL KM_SPI_DATA_NOPULL

/**
 * SPI() constructor
 */
JERRYXX_FUN(spi_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");

  // read parameters
  uint8_t bus = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t mode = SPI_DEFAULT_MODE;
  uint32_t baudrate = SPI_DEFAULT_BAUDRATE;
  uint8_t bitorder = SPI_DEFAULT_BITORDER;
  km_spi_pullup_t pullup = SPI_DEFAULT_PULL;
  km_spi_pins_t def_pins = km_spi_get_default_pins(bus);
  km_spi_pins_t pins = {
      .miso = def_pins.miso,
      .mosi = def_pins.mosi,
      .sck = def_pins.sck,
  };
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t options = JERRYXX_GET_ARG(1);
    mode = (uint8_t)jerryxx_get_property_number(options, MSTR_SPI_MODE,
                                                SPI_DEFAULT_MODE);
    baudrate = (uint32_t)jerryxx_get_property_number(options, MSTR_SPI_BAUDRATE,
                                                     SPI_DEFAULT_BAUDRATE);
    bitorder = (uint8_t)jerryxx_get_property_number(options, MSTR_SPI_BITORDER,
                                                    SPI_DEFAULT_BITORDER);
    pullup = (uint8_t)jerryxx_get_property_number(options, MSTR_SPI_PULLUP,
                                                    SPI_DEFAULT_PULL);
    pins.miso = (int8_t)jerryxx_get_property_number(options, MSTR_SPI_MISO,
                                                    def_pins.miso);
    pins.mosi = (int8_t)jerryxx_get_property_number(options, MSTR_SPI_MOSI,
                                                    def_pins.mosi);
    pins.sck = (int8_t)jerryxx_get_property_number(options, MSTR_SPI_SCK,
                                                   def_pins.sck);
  }

  if (bitorder != KM_SPI_BITORDER_LSB) bitorder = KM_SPI_BITORDER_MSB;
  if (mode < 0 || mode > 3)
    return jerry_error_sz(JERRY_ERROR_RANGE,
                              "SPI mode error.");
  // initialize the bus
  int ret = km_spi_setup(bus, (km_spi_mode_t)mode, baudrate,
                         (km_spi_bitorder_t)bitorder, pins, pullup);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  } else {
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BUS, bus);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_MODE, mode);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BAUDRATE, baudrate);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_BITORDER, bitorder);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_MISO, pins.miso);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_MOSI, pins.mosi);
    jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_SPI_SCK, pins.sck);
    return jerry_undefined();
  }
}

/**
 * SPI.prototype.transfer() function
 */
JERRYXX_FUN(spi_transfer_fn) {
  JERRYXX_CHECK_ARG(0, "data");

  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);

  // write data to the bus
  if (jerry_value_is_typedarray(data) &&
      jerry_typedarray_type(data) ==
          JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_arraybuffer_size(array_buffer);
    uint8_t *tx_buf = jerry_arraybuffer_data(array_buffer);
    uint8_t *rx_buf = malloc(len);
    int ret = km_spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    jerry_value_free(array_buffer);
    if (ret < 0) {
      free(rx_buf);
      return jerry_exception_value(create_system_error(ret), true);
    } else {
      jerry_value_t buffer =
          jerry_arraybuffer_external(rx_buf, len, NULL);
      jerry_value_t array = jerry_typedarray_with_buffer(
          JERRY_TYPEDARRAY_UINT8, buffer);
      jerry_value_free(buffer);
      return array;
    }
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t tx_buf[len];
    uint8_t *rx_buf = malloc(len);
    jerryxx_string_to_ascii_char_buffer(data, tx_buf, len);
    int ret = km_spi_sendrecv(bus, tx_buf, rx_buf, len, timeout);
    if (ret < 0) {
      free(rx_buf);
      return jerry_exception_value(create_system_error(ret), true);
    } else {
      jerry_value_t buffer =
          jerry_arraybuffer_external(rx_buf, len, NULL);
      jerry_value_t array = jerry_typedarray_with_buffer(
          JERRY_TYPEDARRAY_UINT8, buffer);
      jerry_value_free(buffer);
      return array;
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The data argument must be Uint8Array or string.");
  }
}

/**
 * SPI.prototype.send() function
 */
JERRYXX_FUN(spi_send_fn) {
  JERRYXX_CHECK_ARG(0, "data");

  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);
  uint32_t count = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(2, 1);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);

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
    uint8_t *tx_buf = jerry_arraybuffer_data(array_buffer);
    for (int c = 0; c < count; c++) {
      ret = km_spi_send(bus, tx_buf, len, timeout);
      if (ret < 0) break;
    }
    jerry_value_free(array_buffer);
  } else if (jerry_value_is_string(data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(data);
    uint8_t tx_buf[len];
    jerryxx_string_to_ascii_char_buffer(data, tx_buf, len);
    for (int c = 0; c < count; c++) {
      ret = km_spi_send(bus, tx_buf, len, timeout);
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
 * SPI.prototype.recv() function
 */
JERRYXX_FUN(spi_recv_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "length");
  uint32_t length = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  uint32_t timeout = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(1, 5000);

  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);

  // recv data
  uint8_t *buf = malloc(length);
  int ret = km_spi_recv(bus, 0, buf, length, timeout);

  // return an Uin8Array
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
 * SPI.prototype.close() function
 */
JERRYXX_FUN(spi_close_fn) {
  // check this.bus number
  jerry_value_t bus_value =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);
  if (!jerry_value_is_number(bus_value)) {
    jerry_value_free(bus_value);
    return jerry_error_sz(
        JERRY_ERROR_REFERENCE,
        "SPI bus is not initialized.");
  }
  uint8_t bus = (uint8_t)jerry_value_as_number(bus_value);
  jerry_value_free(bus_value);

  // close the bus
  int ret = km_spi_close(bus);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // delete this.bus property
  jerryxx_delete_property(JERRYXX_GET_THIS, MSTR_SPI_BUS);

  return jerry_undefined();
}

/**
 * Initialize 'spi' module
 */
jerry_value_t module_spi_init() {
  /* SPI class */
  jerry_value_t spi_ctor = jerry_function_external(spi_ctor_fn);
  jerry_value_t spi_prototype = jerry_object();
  jerryxx_set_property(spi_ctor, "prototype", spi_prototype);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE0, KM_SPI_MODE_0);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE1, KM_SPI_MODE_1);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE2, KM_SPI_MODE_2);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MODE3, KM_SPI_MODE_3);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_DATA_NOPULL, KM_SPI_DATA_NOPULL);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MOSI_PULLUP, KM_SPI_MOSI_PULLUP);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MISO_PULLUP, KM_SPI_MISO_PULLUP);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_DATA_PULLUP, KM_SPI_DATA_PULLUP);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_MSB, KM_SPI_BITORDER_MSB);
  jerryxx_set_property_number(spi_ctor, MSTR_SPI_LSB, KM_SPI_BITORDER_LSB);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_TRANSFER,
                                spi_transfer_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_SEND, spi_send_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_RECV, spi_recv_fn);
  jerryxx_set_property_function(spi_prototype, MSTR_SPI_CLOSE, spi_close_fn);
  jerry_value_free(spi_prototype);

  /* spi module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_SPI_SPI, spi_ctor);
  jerry_value_free(spi_ctor);

  return exports;
}
