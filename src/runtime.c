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
  jerry_value_t err_val = jerry_get_value_from_error (error_value, false);
  jerry_value_t err_str = jerry_value_to_string (err_val);
  repl_print_begin(REPL_OUTPUT_ERROR);
  repl_print_value("%s\r\n", err_str);
  repl_print_end();
  jerry_release_value (err_val);
  jerry_release_value (err_str);

  // backtrace test
  /*
  jerry_value_t backtrace_array = jerry_get_backtrace (5);
  uint32_t array_length = jerry_get_array_length (backtrace_array);
  for (uint32_t idx = 0; idx < array_length; idx++) {
    jerry_value_t property = jerry_get_property_by_index (backtrace_array, idx);
    jerry_char_t string_buffer[64];
    jerry_size_t copied_bytes = jerry_substring_to_char_buffer (property,
                                                                0,
                                                                63,
                                                                string_buffer,
                                                                63);
    string_buffer[copied_bytes] = '\0';
    printf(" %d: %s\n", idx, string_buffer);
    jerry_release_value (property);
  }
  jerry_release_value (backtrace_array);
  */
  // end of backtrace test
}

void runtime_run_main() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (NULL, 0, script, size, JERRY_PARSE_STRICT_MODE);
    if (!jerry_value_is_error (parsed_code)) {
      jerry_value_t ret_value = jerry_run (parsed_code);
      if (jerry_value_is_error (ret_value)) {
        print_unhandled_exception (ret_value);
      }
      jerry_release_value (ret_value);
    } else {
      print_unhandled_exception (parsed_code);
    }
    jerry_release_value (parsed_code);
  }
}
