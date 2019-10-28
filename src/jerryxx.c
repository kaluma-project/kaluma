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
#include "repl.h"

void jerryxx_set_property(jerry_value_t object, const char *name, jerry_value_t value) {
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, value);
  jerry_release_value(ret);
  jerry_release_value (prop);
}

void jerryxx_set_property_number(jerry_value_t object, const char *name, double value) {
  jerry_value_t val = jerry_create_number(value);
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, val);
  jerry_release_value(ret);
  jerry_release_value (prop);
  jerry_release_value(val);
}

void jerryxx_set_property_string(jerry_value_t object, const char *name, char *value) {
  jerry_value_t val = jerry_create_string((const jerry_char_t *) value);
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, val);
  jerry_release_value(ret);
  jerry_release_value (prop);
  jerry_release_value(val);
}

void jerryxx_set_property_function(jerry_value_t object, const char *name, jerry_external_handler_t fn) {
  jerry_value_t ext_fn = jerry_create_external_function(fn);
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_set_property (object, prop, ext_fn);
  jerry_release_value(ret);
  jerry_release_value (prop);
  jerry_release_value(ext_fn);
}

jerry_value_t jerryxx_get_property(jerry_value_t object, const char *name) {
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_get_property (object, prop);
  jerry_release_value (prop);
  return ret;
}

double jerryxx_get_property_number(jerry_value_t object, const char *name, double default_value) {
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  jerry_value_t ret = jerry_get_property (object, prop);
  double value = default_value;
  if (jerry_value_is_number(ret)) {
    value = jerry_get_number_value(ret);
  }
  jerry_release_value(ret);
  jerry_release_value (prop);
  return value;
}

bool jerryxx_delete_property(jerry_value_t object, const char *name) {
  jerry_value_t prop = jerry_create_string((const jerry_char_t *) name);
  bool ret = jerry_delete_property(object, prop);
  jerry_release_value (prop);
  return ret;
}

void jerryxx_print_value(jerry_value_t value) {
  jerry_value_t str = jerry_value_to_string(value);
  jerry_size_t str_sz = jerry_get_string_size (str);
  jerry_char_t str_buf[str_sz + 1];
  jerry_string_to_char_buffer (str, str_buf, str_sz);
  for (int16_t i = 0; i < str_sz; i++)
    tty_putc(str_buf[i]);
  jerry_release_value(str);
}

/**
 * Print error with stacktrace
 */
void jerryxx_print_error (jerry_value_t value, bool print_stacktrace) {
  jerry_value_t error_value = jerry_get_value_from_error (value, false);
  // print error message  
  jerry_value_t err_str = jerry_value_to_string (error_value);
  repl_set_output(REPL_OUTPUT_ERROR);
  repl_print_value(err_str);
  repl_println();
  repl_set_output(REPL_OUTPUT_NORMAL);
  jerry_release_value (err_str);
  // print stack trace
  if (print_stacktrace && jerry_value_is_object (error_value)) {
    jerry_value_t stack_str = jerry_create_string ((const jerry_char_t *) "stack");
    jerry_value_t backtrace_val = jerry_get_property (error_value, stack_str);
    jerry_release_value (stack_str);
    if (!jerry_value_is_error (backtrace_val)
        && jerry_value_is_array (backtrace_val)) {
      uint32_t length = jerry_get_array_length (backtrace_val);
      if (length > 32) { length = 32; } /* max length: 32 */
      for (uint32_t i = 0; i < length; i++) {
        jerry_value_t item_val = jerry_get_property_by_index (backtrace_val, i);
        if (!jerry_value_is_error (item_val)
            && jerry_value_is_string (item_val)) {
          repl_set_output(REPL_OUTPUT_ERROR);
          repl_printf("  at ");
          repl_print_value(item_val);
          repl_println();
          repl_set_output(REPL_OUTPUT_NORMAL);
        }
        jerry_release_value (item_val);        
      }
    }
    jerry_release_value (backtrace_val);
  }
  jerry_release_value (error_value);
}
