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

#include <string.h>
#include "tty.h"
#include "flash.h"
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "global.h"
#include "repl.h"
#include "runtime.h"

// --------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------

static void print_value_in_format(const char *format, jerry_value_t value) {
  jerry_value_t str = jerry_value_to_string(value);
  jerry_size_t str_sz = jerry_get_string_size (str);
  jerry_char_t str_buf[str_sz + 1];
  jerry_string_to_char_buffer (str, str_buf, str_sz);
  str_buf[str_sz] = '\0';
  tty_printf(format, (char *) str_buf);
}

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

void runtime_init() {
  jerry_init (JERRY_INIT_EMPTY);
  global_init();
  jerry_gc();
  runtime_run_main();  
}

void runtime_deinit() {
  jerry_cleanup ();  
}

void runtime_run_main() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (script, size, false);
    if (!jerry_value_has_error_flag (parsed_code))
    {
      jerry_value_t ret_value = jerry_run (parsed_code);
      jerry_release_value (ret_value);
    }
    jerry_release_value (parsed_code);  
  }
}

void runtime_print_value(const jerry_value_t value, int depth) {
  if (jerry_value_is_array(value)) {
    if (depth == 0) {
      repl_printf("[Array]");
    } else {
      uint32_t len = jerry_get_array_length(value);
      repl_printf("[");
      for (int i = 0; i < len; i++) {
        jerry_value_t item = jerry_get_property_by_index(value, i);
        if (i > 0) {
          repl_printf(", ");
        } else {
          repl_printf(" ");
        }
        runtime_print_value(item, depth - 1);
        jerry_release_value(item);
      }
      repl_printf(" ]");
    }
  } else if (jerry_value_is_boolean(value)) {
    print_value_in_format("%s", value);
  } else if (jerry_value_is_function(value)) {
    repl_printf("[Function]");
  } else if (jerry_value_is_constructor(value)) {
    print_value_in_format("[Constructor]", value);
  } else if (jerry_value_is_number(value)) {
    print_value_in_format("%s", value);
  } else if (jerry_value_is_null(value)) {
    repl_printf("null");
  } else if (jerry_value_is_object(value)) {
    if (depth == 0) {
      repl_printf("[Object]");
    } else {
      repl_printf("{");
      jerry_value_t keys = jerry_get_object_keys(value);
      uint32_t len = jerry_get_array_length(keys);
      for (int i = 0; i < len; i++) {
        jerry_value_t prop_name = jerry_get_property_by_index(keys, i);
        jerry_value_t prop_val = jerry_get_property (value, prop_name);
        if (i > 0) {
          repl_printf(", ");
        } else {
          repl_printf(" ");
        }
        print_value_in_format("%s: ", prop_name);
        runtime_print_value(prop_val, depth - 1);
        jerry_release_value(prop_val);
        jerry_release_value(prop_name);
      }
      jerry_release_value(keys);
      repl_printf(" }");
    }
  } else if (jerry_value_is_string(value)) {
    print_value_in_format("\'%s\'", value);
  } else if (jerry_value_is_undefined(value)) {
    repl_printf("undefined");
  } else {
    print_value_in_format("%s", value);
  }
}
