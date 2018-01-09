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
#include "magic_strings.h"
#include "repl.h"
#include "io.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "board.h"
#include "kameleon_config.h"

static void register_global_objects() {
  jerry_value_t global_object = jerry_get_global_object ();
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) MSTR_GLOBAL);
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
  jerryxx_set_property_number(global, MSTR_HIGH, GPIO_HIGH);
  jerryxx_set_property_number(global, MSTR_LOW, GPIO_LOW);
  jerryxx_set_property_number(global, MSTR_INPUT, (double) GPIO_IO_MODE_INPUT);
  jerryxx_set_property_number(global, MSTR_OUTPUT, (double) GPIO_IO_MODE_OUTPUT);
  jerryxx_set_property_number(global, MSTR_CHANGE, (double) IO_WATCH_MODE_CHANGE);
  jerryxx_set_property_number(global, MSTR_RISING, (double) IO_WATCH_MODE_RISING);
  jerryxx_set_property_number(global, MSTR_FALLING, (double) IO_WATCH_MODE_FALLING);
  jerryxx_set_property_function(global, MSTR_PIN_MODE, pin_mode_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_READ, digital_read_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_WRITE, digital_write_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_TOGGLE, digital_toggle_fn);
  jerryxx_set_property_function(global, MSTR_SET_WATCH, set_watch_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_WATCH, clear_watch_fn);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                           ANALOG I/O FUNCTIONS                           */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(analog_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  adc_setup(pin);
  delay(1); // To prevent issue #55
  double value = adc_read(pin);
  return jerry_create_number(value);
}

JERRYXX_FUN(analog_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "frequency");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  double value = JERRYXX_GET_ARG_NUMBER_OPT(1, 0.5);
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(2, 490); // Default 490Hz
  pwm_setup(pin, frequency, value);
  pwm_start(pin);
  return jerry_create_undefined();
}

static void tone_timeout_cb(io_timer_handle_t *timer) {
  uint8_t pin = timer->tag;
  pwm_stop(pin);
}

JERRYXX_FUN(tone_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "frequency");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "duration");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "duty");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(1, 261.626); // C key frequency
  uint32_t duration = (uint32_t) JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  double duty = JERRYXX_GET_ARG_NUMBER_OPT(3, 0.5);
  pwm_setup(pin, frequency, duty);
  pwm_start(pin);
  // setup timer for duration
  if (duration > 0) {
    io_timer_handle_t *timer = malloc(sizeof(io_timer_handle_t));
    io_timer_init(timer);
    timer->tag = pin;
    io_timer_start(timer, tone_timeout_cb, duration, false);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(no_tone_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  pwm_stop(pin);
  return jerry_create_undefined();
}

static void register_global_analog_io() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_function(global, MSTR_ANALOG_READ, analog_read_fn);
  jerryxx_set_property_function(global, MSTR_ANALOG_WRITE, analog_write_fn);
  jerryxx_set_property_function(global, MSTR_TONE, tone_fn);
  jerryxx_set_property_function(global, MSTR_NO_TONE, no_tone_fn);
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
  jerryxx_set_property_function(global, MSTR_SET_TIMEOUT, set_timeout_fn);
  jerryxx_set_property_function(global, MSTR_SET_INTERVAL, set_interval_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_TIMEOUT, clear_timer_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_INTERVAL, clear_timer_fn);
  jerryxx_set_property_function(global, MSTR_DELAY, delay_fn);
  jerryxx_set_property_function(global, MSTR_MILLIS, millis_fn);
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
  jerryxx_set_property_function(console, MSTR_LOG, console_log_fn);
  jerryxx_set_property_function(console, MSTR_ERROR, console_error_fn);
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, MSTR_CONSOLE, console);
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
  jerryxx_set_property_string(process, MSTR_ARCH, board_arch);
  jerryxx_set_property_string(process, MSTR_PLATFORM, board_platform);
  jerryxx_set_property_string(process, MSTR_VERSION, CONFIG_KAMELEON_VERSION);

  /* Add `process.binding` function and it's properties */
  jerry_value_t binding_fn = jerry_create_external_function(process_binding_fn);
  jerry_value_t binding_prop = jerry_create_string((const jerry_char_t *) MSTR_BINDING);
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
  jerry_value_t prop_buildin_modules = jerry_create_string((const jerry_char_t *) MSTR_BUILTIN_MODULES);
  jerry_set_property(process, prop_buildin_modules, array_modules);
  jerry_release_value(prop_buildin_modules);
  jerry_release_value(array_modules);

  /* Add `process.getBuiltinModule` function */
  jerryxx_set_property_function(process, MSTR_GET_BUILTIN_MODULE, process_get_builtin_module_fn);

  /* Register 'process' object to global */
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, MSTR_PROCESS, process);  

  jerry_release_value(process);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                               BOARD OBJECT                               */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(board_led_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "index");
  int index = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  JERRYXX_CHECK_INDEX_RANGE(index, 0, led_num-1)
  return jerry_create_number(led_pins[index]);
}

JERRYXX_FUN(board_switch_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "index");
  int index = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  JERRYXX_CHECK_INDEX_RANGE(index, 0, switch_num-1)
  return jerry_create_number(switch_pins[index]);
}

JERRYXX_FUN(board_pwm_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "index");
  int index = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  JERRYXX_CHECK_INDEX_RANGE(index, 0, pwm_num-1)
  return jerry_create_number(pwm_pins[index]);
}

JERRYXX_FUN(board_adc_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "index");
  int index = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  JERRYXX_CHECK_INDEX_RANGE(index, 0, adc_num-1)
  return jerry_create_number(adc_pins[index]);
}

static void register_global_board_object() {
  jerry_value_t board = jerry_create_object();
  jerryxx_set_property_string(board, MSTR_NAME, board_name);
  jerryxx_set_property_number(board, MSTR_PIN_NUM, pin_num);
  jerryxx_set_property_number(board, MSTR_LED_NUM, led_num);
  jerryxx_set_property_number(board, MSTR_SWITCH_NUM, switch_num);
  jerryxx_set_property_number(board, MSTR_PWM_NUM, pwm_num);
  jerryxx_set_property_number(board, MSTR_ADC_NUM, adc_num);
  jerryxx_set_property_number(board, MSTR_I2C_NUM, i2c_num);
  jerryxx_set_property_number(board, MSTR_SPI_NUM, spi_num);
  jerryxx_set_property_number(board, MSTR_UART_NUM, uart_num);
  jerryxx_set_property_function(board, MSTR_LED, board_led_fn);
  jerryxx_set_property_function(board, MSTR_SWITCH, board_switch_fn);
  jerryxx_set_property_function(board, MSTR_PWM, board_pwm_fn);
  jerryxx_set_property_function(board, MSTR_ADC, board_adc_fn);
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_object(global, MSTR_BOARD, board);
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
  register_global_analog_io();
  register_global_timers();
  register_global_console_object();
  register_global_process_object();
  register_global_board_object();
  run_startup_module();
}
