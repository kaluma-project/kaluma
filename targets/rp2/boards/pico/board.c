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

#include "board.h"

#include <stdlib.h>

#include "err.h"
#include "flash2.h"
#include "jerryscript.h"
#include "jerryxx.h"

/**
 * FlashBD (block device) constructor
 * args:
 *   base {number} base sector number
 *   count (number)
 */
JERRYXX_FUN(flashbd_ctor_fn) {
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
  return jerry_create_undefined();
}

/**
 * FlashBD.prototype.read()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(flashbd_read_fn) {
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
      jerry_get_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_get_arraybuffer_pointer(arrbuf);
  jerry_release_value(arrbuf);

  // read from flash
  int base = jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);
  int size = jerryxx_get_property_number(JERRYXX_GET_THIS, "size", 0);
  for (int i = 0; i < buffer_length; i++) {
    buffer_pointer[i] = flash_target[((base + block) * size) + offset + i];
  }
  return jerry_create_undefined();
}

/**
 * FlashBD.prototype.write()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(flashbd_write_fn) {
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
      jerry_get_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_get_arraybuffer_pointer(arrbuf);
  jerry_release_value(arrbuf);

  // write to buffer
  int base = jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);
  km_flash2_program(base + block, offset, buffer_pointer, buffer_length);
  return jerry_create_undefined();
}

/**
 * FlashBD.prototype.ioctl()
 * args:
 *   op {number}
 *   arg {number}
 */
JERRYXX_FUN(flashbd_ioctl_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "op")
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "arg")
  int op = JERRYXX_GET_ARG_NUMBER(0);
  int arg = JERRYXX_GET_ARG_NUMBER_OPT(1, 0);
  int base = (int)jerryxx_get_property_number(JERRYXX_GET_THIS, "base", 0);

  switch (op) {
    case 1:  // init
      return jerry_create_number(0);
    case 2:  // shutdown
      return jerry_create_number(0);
    case 3:  // sync
      return jerry_create_number(0);
    case 4:  // block count
      return jerry_create_number(
          jerryxx_get_property_number(JERRYXX_GET_THIS, "count", 0));
    case 5:  // block size
      return jerry_create_number(
          jerryxx_get_property_number(JERRYXX_GET_THIS, "size", 0));
    case 6:  // erase block
      km_flash2_erase(base + arg, 1);
      return jerry_create_number(0);
    case 7:                             // buffer size
      return jerry_create_number(256);  // flash page size
    default:
      return jerry_create_number(-1);
  }
}

/**
 * Initialize board
 */
void board_init() {
  /* FlashBD class */
  jerry_value_t flashbd_ctor = jerry_create_external_function(flashbd_ctor_fn);
  jerry_value_t flashbd_prototype = jerry_create_object();
  jerryxx_set_property(flashbd_ctor, "prototype", flashbd_prototype);
  jerryxx_set_property_function(flashbd_prototype, "read", flashbd_read_fn);
  jerryxx_set_property_function(flashbd_prototype, "write", flashbd_write_fn);
  jerryxx_set_property_function(flashbd_prototype, "ioctl", flashbd_ioctl_fn);
  jerry_release_value(flashbd_prototype);

  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property(global, "FlashBD", flashbd_ctor);
  jerry_release_value(global);
  jerry_release_value(flashbd_ctor);
}
