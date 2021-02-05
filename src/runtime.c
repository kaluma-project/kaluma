/* Copyright (c) 2017 Kalamu
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

#ifdef _TARGET_FREERTOS_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#include <stdlib.h>
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
#include "kalamu_magic_strings.h"
#include "jerryxx.h"


// --------------------------------------------------------------------------
// PRIVATE VARIABLES
// --------------------------------------------------------------------------

/**
 * Runtime VM stop
 * - 0: normal
 * - 1: break VM execution 
 */
static uint8_t runtime_vm_stop = 0;

/**
 * idle handle for processing enqueued jobs
 */
static io_idle_handle_t idler;

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

static void idler_cb() {
  jerry_value_t ret_val = jerry_run_all_enqueued_jobs();
  if (jerry_value_is_error(ret_val)) {
    jerryxx_print_error(ret_val, true);
  }
  jerry_release_value(ret_val);
#ifdef _TARGET_FREERTOS_  
  // ESP32 Kick the dog
  vTaskDelay(10);
#endif
}

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

void runtime_init(bool load, bool first) {
  jerry_init (JERRY_INIT_EMPTY);
  jerry_set_vm_exec_stop_callback (vm_exec_stop_callback, &runtime_vm_stop, 16);
  jerry_register_magic_strings (magic_string_items, num_magic_string_items, magic_string_lengths);
  global_init();
  jerry_gc(JERRY_GC_PRESSURE_HIGH);
  if (load) {
    runtime_load();
  }
  if (first) {
    // Initialize idler handle for queued jobs in jerryscript
    io_idle_init(&idler);
    io_idle_start(&idler, idler_cb);
  }
}

void runtime_cleanup() {
  jerry_cleanup();
  system_cleanup();
  io_timer_cleanup();
  io_watch_cleanup();
  io_uart_cleanup();
  // io_idle_cleanup();
  // Do not cleanup tty I/O to keep terminal communication
}

void runtime_load() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (NULL, 0, script, size, JERRY_PARSE_STRICT_MODE);
    flash_free_data(script);
    if (!jerry_value_is_error (parsed_code)) {
      jerry_value_t ret_value = jerry_run (parsed_code);
      if (jerry_value_is_error (ret_value)) {
        jerryxx_print_error(ret_value, true);
        runtime_cleanup();
        runtime_init(false, false);
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
