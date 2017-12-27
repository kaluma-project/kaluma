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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jerryscript.h"
#include "jerryxx.h"
#include "tty.h"

void jerryxx_set_propery_number(jerry_value_t object, const char *name, double value) {
  jerry_value_t val = jerry_create_number(value);
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, val);
  jerry_release_value(ret);
  jerry_release_value (prop);
  jerry_release_value(val);
}

void jerryxx_set_propery_object(jerry_value_t object, const char *name, jerry_value_t obj) {
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, obj);
  jerry_release_value(ret);
  jerry_release_value (prop);
}

void jerryxx_set_propery_function(jerry_value_t object, const char *name, jerry_external_handler_t fn) {
  jerry_value_t ext_fn = jerry_create_external_function(fn);
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, ext_fn);
  jerry_release_value(ret);
  jerry_release_value (prop);
  jerry_release_value(ext_fn);
}

void jerryxx_print_value(const char *format, jerry_value_t value) {
  jerry_value_t str = jerry_value_to_string(value);
  jerry_size_t str_sz = jerry_get_string_size (str);
  jerry_char_t str_buf[str_sz + 1];
  jerry_string_to_char_buffer (str, str_buf, str_sz);
  str_buf[str_sz] = '\0';
  tty_printf(format, (char *) str_buf);
}
