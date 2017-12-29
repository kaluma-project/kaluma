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

#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "runtime.h"
#include "global.h"
#include "jerryxx.h"
#include "kameleon_modules.h"
#include "repl.h"
#include "io.h"
#include "gpio.h"
#include "board.h"

static void register_global_objects() {
  jerry_value_t global_object = jerry_get_global_object ();
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "global");
  jerry_set_property (global_object, prop_name, global_object);
  jerry_release_value (prop_name);
  jerry_release_value (global_object);
}

/****************************************************************************/
/*                                                                          */
/*                          DIGITAL I/O FUNCTIONS                           */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(pin_mode_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "mode");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  gpio_io_mode_t mode = (gpio_io_mode_t) JERRYXX_GET_ARG_NUMBER_OPT(1, GPIO_IO_MODE_INPUT);
  gpio_set_io_mode(pin, mode);
  return jerry_create_undefined();
}

JERRYXX_FUN(digital_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  uint8_t value = gpio_read(pin);
  return jerry_create_number(value);
}

JERRYXX_FUN(digital_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  uint8_t value = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, GPIO_LOW);
  gpio_write(pin, value);
  return jerry_create_undefined();
}

JERRYXX_FUN(digital_toggle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  gpio_toggle(pin);
  return jerry_create_undefined();
}

static void set_watch_cb(io_watch_handle_t *watch) {
  if (jerry_value_is_function(watch->watch_js_cb)) {
    jerry_value_t this_val = jerry_create_undefined ();
    jerry_value_t ret_val = jerry_call_function (watch->watch_js_cb, this_val, NULL, 0);
    if (!jerry_value_has_error_flag (ret_val)) {
      // TODO: handle error and return value
    }
    jerry_release_value (ret_val);
    jerry_release_value (this_val);
  } 
}

JERRYXX_FUN(set_watch_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "mode");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "debounce");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
  io_watch_mode_t mode = JERRYXX_GET_ARG_NUMBER_OPT(2, IO_WATCH_MODE_CHANGE);
  uint32_t debounce = JERRYXX_GET_ARG_NUMBER_OPT(3, 0);
  io_watch_handle_t *watch = malloc(sizeof(io_watch_handle_t));
  io_watch_init(watch);
  watch->watch_js_cb = callback;
  io_watch_start(watch, set_watch_cb, pin, mode, debounce);
  return jerry_create_number(watch->base.id);
}

static void watch_close_cb(io_handle_t *handle) {
  free(handle);
}

JERRYXX_FUN(clear_watch_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "id");
  int id = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  io_watch_handle_t *watch = io_watch_get_by_id(id);
  if (watch != NULL) {
    io_watch_stop(watch);
    io_handle_close((io_handle_t *) watch, watch_close_cb);
  }
  return jerry_create_undefined();
}

static void register_global_digital_io() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_number(global, "HIGH", GPIO_HIGH);
  jerryxx_set_property_number(global, "LOW", GPIO_LOW);
  jerryxx_set_property_number(global, "INPUT", (double) GPIO_IO_MODE_INPUT);
  jerryxx_set_property_number(global, "OUTPUT", (double) GPIO_IO_MODE_OUTPUT);
  jerryxx_set_property_number(global, "CHANGE", (double) IO_WATCH_MODE_CHANGE);
  jerryxx_set_property_number(global, "RISING", (double) IO_WATCH_MODE_RISING);
  jerryxx_set_property_number(global, "FALLING", (double) IO_WATCH_MODE_FALLING);
  jerryxx_set_property_function(global, "pinMode", pin_mode_fn);
  jerryxx_set_property_function(global, "digitalRead", digital_read_fn);
  jerryxx_set_property_function(global, "digitalWrite", digital_write_fn);
  jerryxx_set_property_function(global, "digitalToggle", digital_toggle_fn);
  jerryxx_set_property_function(global, "setWatch", set_watch_fn);
  jerryxx_set_property_function(global, "clearWatch", clear_watch_fn);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                              TIMER FUNCTIONS                             */
/*                                                                          */
/****************************************************************************/

static void set_timer_cb(io_timer_handle_t *timer) {
  if (jerry_value_is_function(timer->timer_js_cb)) {
    jerry_value_t this_val = jerry_create_undefined ();
    jerry_value_t ret_val = jerry_call_function (timer->timer_js_cb, this_val, NULL, 0);
    if (!jerry_value_has_error_flag (ret_val)) {
      // handle return value
    }
    jerry_release_value (ret_val);
    jerry_release_value (this_val);
  } 
}

JERRYXX_FUN(set_timeout_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "delay");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint64_t delay = (uint64_t) JERRYXX_GET_ARG_NUMBER(1);  
  io_timer_handle_t *timer = malloc(sizeof(io_timer_handle_t));
  io_timer_init(timer);
  timer->timer_js_cb = callback;
  io_timer_start(timer, set_timer_cb, delay, false);
  return jerry_create_number(timer->base.id);
}

JERRYXX_FUN(set_interval_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "delay");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint64_t delay = (uint64_t) JERRYXX_GET_ARG_NUMBER(1);  
  io_timer_handle_t *timer = malloc(sizeof(io_timer_handle_t));
  io_timer_init(timer);
  timer->timer_js_cb = callback;
  io_timer_start(timer, set_timer_cb, delay, true);
  return jerry_create_number(timer->base.id);
}

static void timer_close_cb(io_handle_t *handle) {
  free(handle);
}

JERRYXX_FUN(clear_timer_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id");
  int id = (int) JERRYXX_GET_ARG_NUMBER(0);
  io_timer_handle_t *timer = io_timer_get_by_id(id);
  if (timer != NULL) {
    io_timer_stop(timer);
    io_handle_close((io_handle_t *) timer, timer_close_cb);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(delay_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "id");
  uint64_t delay_val = (uint64_t) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  delay(delay_val);
  return jerry_create_undefined();
}

JERRYXX_FUN(millis_fn) {
  uint64_t msec = gettime();
  return jerry_create_number(msec);
}

static void register_global_timers() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_function(global, "setTimeout", set_timeout_fn);
  jerryxx_set_property_function(global, "setInterval", set_interval_fn);
  jerryxx_set_property_function(global, "clearTimeout", clear_timer_fn);
  jerryxx_set_property_function(global, "clearInterval", clear_timer_fn);
  jerryxx_set_property_function(global, "delay", delay_fn);
  jerryxx_set_property_function(global, "millis", millis_fn);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                              CONSOLE OBJECT                              */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(console_log_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[0m"); // set to normal color
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value("%s", JERRYXX_GET_ARG(i));
    }
    repl_printf("\r\n");
    repl_print_end();
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(console_error_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    repl_print_begin(REPL_OUTPUT_ERROR);
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[31m"); // red
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value("%s", JERRYXX_GET_ARG(i));
    }
    repl_printf("\r\n");
    repl_printf("\33[0m"); // back to normal color
    repl_print_end();
  }
  return jerry_create_undefined();
}

static void register_global_console_object() {
  jerry_value_t console = jerry_create_object();
  jerryxx_set_property_function(console, "log", console_log_fn);
  jerryxx_set_property_function(console, "error", console_error_fn);
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, "console", console);
  jerry_release_value(console);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                              PROCESS OBJECT                              */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(process_binding_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "native_module_name")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, native_module_name)
  /* Return a native initialized object */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, native_module_name) == 0 && builtin_modules[i].fn != NULL) {
      return builtin_modules[i].fn();
    }
  }
  /* If no corresponding module, return undefined */
  return jerry_create_undefined();
}


JERRYXX_FUN(process_get_builtin_module_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "builtin_module_name")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, builtin_module_name)
  /* Find and return a builtin module */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, builtin_module_name) == 0) {
      jerry_value_t fn = jerry_exec_snapshot(builtin_modules[i].code, builtin_modules[i].size, true);
      return fn;
    }
  }
  /* If no corresponding module, return undefined */
  return jerry_create_undefined();
}

static void register_global_process_object() {
  jerry_value_t process = jerry_create_object();
  jerryxx_set_property_string(process, "arch", board_arch);
  jerryxx_set_property_string(process, "platform", board_platform);
  // TODO: jerryxx_set_property_string(process, "version", version);

  /* Add `process.binding` function and it's properties */
  jerry_value_t binding_fn = jerry_create_external_function(process_binding_fn);
  jerry_value_t binding_prop = jerry_create_string((const jerry_char_t *) "binding");
  jerry_set_property (process, binding_prop, binding_fn);
  jerry_release_value (binding_prop);
  for (int i = 0; i < builtin_modules_length; i++) {
    if (builtin_modules[i].fn != NULL) {
      jerry_value_t value = jerry_create_string((const jerry_char_t *) builtin_modules[i].name);
      jerry_value_t ret = jerry_set_property(binding_fn, value, value);
      jerry_release_value(ret);
      jerry_release_value(value);
    }
  }
  jerry_release_value(binding_fn);

  /* Add `process.buildin_modules` array property */
  jerry_value_t array_modules = jerry_create_array(builtin_modules_length);
  for (int i = 0; i < builtin_modules_length; i++) {
    jerry_value_t value = jerry_create_string((const jerry_char_t *) builtin_modules[i].name);
    jerry_value_t ret = jerry_set_property_by_index(array_modules, i, value);
    jerry_release_value(ret);
    jerry_release_value(value);
  }
  jerry_value_t prop_buildin_modules = jerry_create_string((const jerry_char_t *) "builtin_modules");
  jerry_set_property(process, prop_buildin_modules, array_modules);
  jerry_release_value(prop_buildin_modules);
  jerry_release_value(array_modules);

  /* Add `process.getBuiltinModule` function */
  jerryxx_set_property_function(process, "getBuiltinModule", process_get_builtin_module_fn);

  /* Register 'process' object to global */
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, "process", process);  

  jerry_release_value(process);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                               BOARD OBJECT                               */
/*                                                                          */
/****************************************************************************/

static void register_global_board_object() {
  jerry_value_t board = jerry_create_object();
  jerryxx_set_property_string(board, "name", board_name);
  // ...
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, "board", board);
  jerry_release_value(board);
  jerry_release_value(global);
}


static void run_startup_module() {
  jerry_value_t res = jerry_exec_snapshot(module_startup_code, module_startup_size, false);
  jerry_value_t this_val = jerry_create_undefined ();
  jerry_value_t ret_val = jerry_call_function (res, this_val, NULL, 0);
  jerry_release_value (ret_val);
  jerry_release_value (this_val);
  jerry_release_value (res);  
}

void global_init() {
  register_global_objects();
  register_global_digital_io();
  register_global_timers();
  register_global_console_object();
  register_global_process_object();
  register_global_board_object();
  run_startup_module();
}
