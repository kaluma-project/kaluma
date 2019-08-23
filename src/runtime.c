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
#include "jerryxx.h"

/**
 * Runtime VM stop
 * - 0: normal
 * - 1: break VM execution 
 */
static uint8_t runtime_vm_stop = 0;

// --------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------

static jerry_value_t vm_exec_stop_callback (void *user_p) {
  if (runtime_vm_stop > 0) {
    runtime_vm_stop = 0;
    return jerry_create_string ((const jerry_char_t *) "Abort script"); 
  }
  return jerry_create_undefined ();
}

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

void runtime_init(bool run_main) {
  jerry_init (JERRY_INIT_EMPTY);
  jerry_set_vm_exec_stop_callback (vm_exec_stop_callback, &runtime_vm_stop, 16);
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

void runtime_run_main() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (NULL, 0, script, size, JERRY_PARSE_STRICT_MODE);
    if (!jerry_value_is_error (parsed_code)) {
      jerry_value_t ret_value = jerry_run (parsed_code);
      if (jerry_value_is_error (ret_value)) {
        jerryxx_print_error(ret_value, true);
        runtime_cleanup();
        runtime_init(false);
        return;
      }
      jerry_release_value (ret_value);
    } else {
      jerryxx_print_error(parsed_code, true);
    }
    jerry_release_value (parsed_code);
  }
}

void runtime_set_vm_stop(uint8_t stop) {
  runtime_vm_stop = stop;
}
