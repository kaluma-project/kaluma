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
#include "jerryscript-port.h"
#include "jerryscript-ext/handler.h"
#include "io.h"
#include "gpio.h"
#include "global.h"
#include "repl.h"
#include "system.h"
#include "runtime.h"
#include "kameleon_magic_strings.h"

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

void runtime_init(bool run_main) {
  jerry_init (JERRY_INIT_EMPTY);
  jerry_register_magic_strings (magic_string_items, num_magic_string_items, magic_string_lengths);
  global_init();
  jerry_gc(JERRY_GC_PRESSURE_HIGH);
  if (run_main) {
    runtime_run_main();
  }
}

void runtime_cleanup() {
  jerry_cleanup();
  system_cleanup();
  io_timer_cleanup();
  io_watch_cleanup();
  io_uart_cleanup();
  // Do not cleanup tty I/O to keep terminal communication
}

/**
 * Print error value
 */
static void print_unhandled_exception (jerry_value_t error_value) {
  // print error message  
  jerry_value_t err_str = jerry_value_to_string (error_value);
  repl_print_begin(REPL_OUTPUT_ERROR);
  repl_print_value("%s\r\n", err_str);
  repl_print_end();
  jerry_release_value (err_str);

  // print stack trace
  if (jerry_value_is_object (error_value)) {
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
          repl_print_begin(REPL_OUTPUT_ERROR);
          repl_print_value("  at %s\r\n", item_val);
          repl_print_end();          
        }
        jerry_release_value (item_val);        
      }
    }
    jerry_release_value (backtrace_val);
  }
}

void runtime_run_main() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (NULL, 0, script, size, JERRY_PARSE_STRICT_MODE);
    if (!jerry_value_is_error (parsed_code)) {
      jerry_value_t ret_value = jerry_run (parsed_code);
      if (jerry_value_is_error (ret_value)) {
        jerry_value_t error_value = jerry_get_value_from_error (ret_value, true);
        print_unhandled_exception (error_value);
        jerry_release_value (error_value);
      }
      jerry_release_value (ret_value);
    } else {
      jerry_value_t error_value = jerry_get_value_from_error (parsed_code, true);
      print_unhandled_exception (error_value);
      jerry_release_value (error_value);
    }
    jerry_release_value (parsed_code);
  }
}
