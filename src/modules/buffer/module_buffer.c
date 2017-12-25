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
#include "module_buffer.h"

jerry_value_t ctor;

static void native_buffer_free_cb(void *native_p) {
  native_buffer_t *native_buf = (native_buffer_t *) native_p;
  if (native_buf->size > 0) {
    free(native_buf->buf);
  }
  free(native_buf);
}

static const jerry_object_native_info_t native_buffer_type_info =
{
  .free_cb = native_buffer_free_cb
};

static jerry_value_t buffer_ctor(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  return jerry_create_undefined();
}

static jerry_value_t buffer_alloc(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 2);
  // ASSERT(jerry_value_is_number(args_p[0]))
  // ASSERT(jerry_value_is_number(args_p[1]))
  int size = (int) jerry_get_number_value(args_p[0]);
  uint8_t fill = (uint8_t) jerry_get_number_value(args_p[1]);
  // create buffer object  
  jerry_value_t object = jerry_construct_object(ctor, NULL, 0);
  native_buffer_t *native_buf = malloc(sizeof(native_buffer_t));
  native_buf->size = size;
  native_buf->buf = malloc(size);

  /* TODO: Fill the buf with `fill` param */

  jerry_set_object_native_pointer (object, native_buf, &native_buffer_type_info);

  jerryxx_set_propery_number("length", size);

  return object;
}

jerry_value_t module_buffer_init() {
  ctor = jerry_create_external_function(buffer_ctor);

  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_propery_object(ctor, "prototype", prototype);
  jerry_release_value (prototype);

  jerryxx_set_propery_function(ctor, "alloc", buffer_alloc);
  return ctor;
}
