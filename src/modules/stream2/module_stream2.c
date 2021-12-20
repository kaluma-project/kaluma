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
#include "stream2_magic_strings.h"

/**
 * Stream() constructor
 */
JERRYXX_FUN(stream_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");
}

/**
 * Stream.prototype.method()
 */
JERRYXX_FUN(stream_method_fn) {}

/**
 * Initialize 'stream2' module
 */
jerry_value_t module_stream2_init() {
  /* Stream class */
  jerry_value_t stream_ctor = jerry_create_external_function(stream_ctor_fn);
  jerry_value_t stream_prototype = jerry_create_object();
  jerryxx_set_property(stream_ctor, "prototype", stream_prototype);
  jerryxx_set_property_function(stream_prototype, "method", stream_method_fn);
  // ...
  jerry_release_value(stream_prototype);
  // TODO: extends EventEmitter -- jerryxx_extends(subclass, superclass))

  /* stream module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_STREAM2_STREAM, stream_ctor);
  jerry_release_value(stream_ctor);
  return exports;
}
