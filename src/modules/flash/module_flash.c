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

#include "module_flash.h"

#include <stdlib.h>

#include "board.h"
#include "err.h"
#include "flash.h"
#include "flash_magic_strings.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"

/**
 * Flash (block device) constructor
 * args:
 *   base {number} base sector number
 *   count (number)
 */
JERRYXX_FUN(flash_ctor_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "base")
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "count")
  int base = JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  int count = JERRYXX_GET_ARG_NUMBER_OPT(1, KALUMA_FLASH_SECTOR_COUNT);
  int size = KALUMA_FLASH_SECTOR_SIZE;

  // set properties to this
  jerryxx_set_property_number(JERRYXX_GET_THIS, "base", base);
  jerryxx_set_property_number(JERRYXX_GET_THIS, "count", count);
  jerryxx_set_property_number(JERRYXX_GET_THIS, "size", size);
  return jerry_undefined();
}

/**
 * Flash.prototype.read()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(flash_read_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "block")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  int block = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  int offset = JERRYXX_GET_ARG_NUMBER_OPT(2, 0);

  // get buffer pointer
  jerry_length_t buffer_length = 0;
  jerry_length_t buffer_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);

  // printf("bd.read(%d, %d, %d)\r\n", block, buffer_length, offset);

  // read from flash
  int base = jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);
  int size = jerryxx_get_property_number(JERRYXX_GET_THIS, "size", 0);
  const uint8_t *addr = km_flash_addr;
  for (int i = 0; i < buffer_length; i++) {
    buffer_pointer[i] = addr[((base + block) * size) + offset + i];
  }
  return jerry_undefined();
}

/**
 * Flash.prototype.write()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(flash_write_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "block")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  int block = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  int offset = JERRYXX_GET_ARG_NUMBER_OPT(2, 0);

  // get buffer pointer
  jerry_length_t buffer_length = 0;
  jerry_length_t buffer_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);

  // printf("bd.write(%d, %d, %d)\r\n", block, buffer_length, offset);

  // write to buffer
  int base = jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);
  km_flash_program(base + block, offset, buffer_pointer, buffer_length);
  return jerry_undefined();
}

/**
 * Flash.prototype.ioctl()
 * args:
 *   op {number}
 *   arg {number}
 */
JERRYXX_FUN(flash_ioctl_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "op")
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "arg")
  int op = JERRYXX_GET_ARG_NUMBER(0);
  int arg = JERRYXX_GET_ARG_NUMBER_OPT(1, 0);
  int base = (int)jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);

  // printf("bd.ioctl(%d, %d)\r\n", op, arg);

  switch (op) {
    case 1:  // init
      return jerry_number(0);
    case 2:  // shutdown
      return jerry_number(0);
    case 3:  // sync
      return jerry_number(0);
    case 4:  // block count
      return jerry_number(
          jerryxx_get_property_number(JERRYXX_GET_THIS, "count", 0));
    case 5:  // block size
      return jerry_number(
          jerryxx_get_property_number(JERRYXX_GET_THIS, "size", 0));
    case 6:  // erase block
      km_flash_erase(base + arg, 1);
      return jerry_number(0);
    case 7:  // buffer size (= flash page size)
      return jerry_number(256);
    default:
      return jerry_number(-1);
  }
}

/**
 * Initialize 'flash' module and return exports
 */
jerry_value_t module_flash_init() {
  /* Flash class */
  jerry_value_t flash_ctor = jerry_function_external(flash_ctor_fn);
  jerry_value_t flash_prototype = jerry_object();
  jerryxx_set_property(flash_ctor, MSTR_PROTOTYPE, flash_prototype);
  jerryxx_set_property_function(flash_prototype, MSTR_FLASH_READ,
                                flash_read_fn);
  jerryxx_set_property_function(flash_prototype, MSTR_FLASH_WRITE,
                                flash_write_fn);
  jerryxx_set_property_function(flash_prototype, MSTR_FLASH_IOCTL,
                                flash_ioctl_fn);
  jerry_value_free(flash_prototype);

  /* flash module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_FLASH_FLASH, flash_ctor);
  jerry_value_free(flash_ctor);
  return exports;
}
