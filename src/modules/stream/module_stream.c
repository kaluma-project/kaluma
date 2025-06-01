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
#include "magic_strings.h"
#include "stream_magic_strings.h"
#include "tty.h"

/**
 * StdInNative() constructor
 */
JERRYXX_FUN(stdin_ctor_fn) { return jerry_undefined(); }

/**
 * StdInNative.prototype.read()
 * args:
 * - size {number}
 * returns: {Uint8Array|null}
 */
JERRYXX_FUN(stdin_read_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "size");
  int size = (int)JERRYXX_GET_ARG_NUMBER_OPT(0, -1);

  // read data from tty (stdin)
  int len = km_tty_available();
  if (len == 0 || len < size) {
    return jerry_null();
  } else {
    if (size < 0) size = len;  // size argument not specified
    jerry_value_t array = jerry_typedarray(JERRY_TYPEDARRAY_UINT8, size);
    jerry_length_t byte_offset = 0;
    jerry_length_t byte_length = 0;
    jerry_value_t buffer =
        jerry_typedarray_buffer(array, &byte_offset, &byte_length);
    uint8_t *buf = jerry_arraybuffer_data(buffer);
    km_tty_read(buf, size);
    jerry_value_free(buffer);
    return array;
  }
}

/**
 * StdOutNative() constructor
 */
JERRYXX_FUN(stdout_ctor_fn) { return jerry_undefined(); }

/**
 * StdOutNative.prototype.write(chunk)
 * args:
 * - data {Uint8Array}
 * returns: {boolean}
 */
JERRYXX_FUN(stdout_write_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_TYPEDARRAY(0, "chunk")
  jerry_value_t chunk = JERRYXX_GET_ARG(0);

  // get array buffer
  jerry_length_t length = 0;
  jerry_length_t offset = 0;
  jerry_value_t arrbuf = jerry_typedarray_buffer(chunk, &offset, &length);
  uint8_t *buf = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  for (int i = 0; i < length; i++) {
    km_tty_putc(buf[i]);
  }
  return jerry_boolean(true);
}

/**
 * Initialize 'stream' module
 */
jerry_value_t module_stream_init() {
  /* StdInNative class */
  jerry_value_t stdin_ctor = jerry_function_external(stdin_ctor_fn);
  jerry_value_t stdin_prototype = jerry_object();
  jerryxx_set_property(stdin_ctor, MSTR_PROTOTYPE, stdin_prototype);
  jerryxx_set_property_function(stdin_prototype, MSTR_STREAM_READ,
                                stdin_read_fn);
  jerry_value_free(stdin_prototype);

  /* StdOutNative class */
  jerry_value_t stdout_ctor = jerry_function_external(stdout_ctor_fn);
  jerry_value_t stdout_prototype = jerry_object();
  jerryxx_set_property(stdout_ctor, MSTR_PROTOTYPE, stdout_prototype);
  jerryxx_set_property_function(stdout_prototype, MSTR_STREAM_WRITE,
                                stdout_write_fn);
  jerry_value_free(stdout_prototype);

  /* stream module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_STREAM_STDIN_NATIVE, stdin_ctor);
  jerryxx_set_property(exports, MSTR_STREAM_STDOUT_NATIVE, stdout_ctor);
  jerry_value_free(stdin_ctor);
  jerry_value_free(stdout_ctor);
  return exports;
}
