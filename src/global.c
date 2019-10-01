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
#include <string.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "runtime.h"
#include "global.h"
#include "jerryxx.h"
#include "kameleon_modules.h"
#include "magic_strings.h"
#include "tty.h"
#include "repl.h"
#include "io.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "system.h"
#include "kameleon_config.h"
#include "base64.h"

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
  if (gpio_set_io_mode(pin, mode) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(digital_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  int value = gpio_read(pin);
  if (value == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_number(value);
}

JERRYXX_FUN(digital_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  uint8_t value = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(1, GPIO_LOW);
  if (gpio_write(pin, value) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(digital_toggle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  if (gpio_toggle(pin) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

static void watch_close_cb(io_handle_t *handle) {
  free(handle);
}

static void set_watch_cb(io_watch_handle_t *watch) {
  if (jerry_value_is_function(watch->watch_js_cb)) {
    jerry_value_t this_val = jerry_create_undefined ();
    jerry_value_t ret_val = jerry_call_function (watch->watch_js_cb, this_val, NULL, 0);
    if (jerry_value_is_error (ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
      // clear handle
      jerry_release_value(watch->watch_js_cb);
      io_watch_stop(watch);
      io_handle_close((io_handle_t *) watch, watch_close_cb);
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
  watch->watch_js_cb = jerry_acquire_value(callback);
  if (io_watch_start(watch, set_watch_cb, pin, mode, debounce) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_number(watch->base.id);
}

JERRYXX_FUN(clear_watch_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "id");
  int id = (int) JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  io_watch_handle_t *watch = io_watch_get_by_id(id);
  if (watch != NULL) {
    jerry_release_value(watch->watch_js_cb);
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
  jerryxx_set_property_number(global, MSTR_INPUT_PULLUP, (double) GPIO_IO_MODE_INPUT_PULLUP);
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
  int adcIndex = adc_setup(pin);
  if (adcIndex == ADCPORT_ERRROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for ADC channel", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  delay(1); // To prevent issue #55
  double value = adc_read((uint8_t) adcIndex);
  return jerry_create_number(value);
}

JERRYXX_FUN(analog_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "frequency");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  double value = JERRYXX_GET_ARG_NUMBER_OPT(1, 0.5);
  if (value < PWM_DUTY_MIN)
    value = PWM_DUTY_MIN;
  else if (value > PWM_DUTY_MAX)
    value = PWM_DUTY_MAX;
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(2, 490); // Default 490Hz
  if (pwm_setup(pin, frequency, value) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for analog out (PWM)", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  } else {
    pwm_start(pin);
    return jerry_create_undefined();
  }
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
  if (duty < PWM_DUTY_MIN)
    duty = PWM_DUTY_MIN;
  else if (duty > PWM_DUTY_MAX)
    duty = PWM_DUTY_MAX;
  if (pwm_setup(pin, frequency, duty) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for tone", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  } else {
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
}

JERRYXX_FUN(no_tone_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  if (pwm_stop(pin) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
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

static void timer_close_cb(io_handle_t *handle) {
  free(handle);
}

static void set_timer_cb(io_timer_handle_t *timer) {
  if (jerry_value_is_function(timer->timer_js_cb)) {
    jerry_value_t this_val = jerry_create_undefined ();
    jerry_value_t ret_val = jerry_call_function (timer->timer_js_cb, this_val, NULL, 0);
    if (jerry_value_is_error(ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
      // clear handle
      jerry_release_value(timer->timer_js_cb);
      io_timer_stop(timer);
      io_handle_close((io_handle_t *) timer, timer_close_cb);
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
  timer->timer_js_cb = jerry_acquire_value(callback);
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
  timer->timer_js_cb = jerry_acquire_value(callback);
  io_timer_start(timer, set_timer_cb, delay, true);
  return jerry_create_number(timer->base.id);
}

JERRYXX_FUN(clear_timer_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id");
  int id = (int) JERRYXX_GET_ARG_NUMBER(0);
  io_timer_handle_t *timer = io_timer_get_by_id(id);
  if (timer != NULL) {
    jerry_release_value(timer->timer_js_cb);
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
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[0m"); // set to normal color
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value(JERRYXX_GET_ARG(i));
    }
    repl_println();
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(console_error_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    repl_set_output(REPL_OUTPUT_ERROR);
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[31m"); // red
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value(JERRYXX_GET_ARG(i));
    }
    repl_println();
    repl_set_output(REPL_OUTPUT_NORMAL);
  }
  return jerry_create_undefined();
}

static void register_global_console_object() {
  jerry_value_t console = jerry_create_object();
  jerryxx_set_property_function(console, MSTR_LOG, console_log_fn);
  jerryxx_set_property_function(console, MSTR_ERROR, console_error_fn);
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property(global, MSTR_CONSOLE, console);
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

JERRYXX_FUN(native_module_wrapper_fn) {
  //jerry_value_t exports = JERRYXX_GET_ARG(0); //comment out because it's not used
  //jerry_value_t require = JERRYXX_GET_ARG(1); //comment out because it's not used
  jerry_value_t module = JERRYXX_GET_ARG(2);
  /* Get module name by module.id */
  jerry_value_t id = jerryxx_get_property(module, MSTR_ID);
  jerry_size_t module_name_sz = jerry_get_string_size(id);
  char module_name[module_name_sz + 1];
  jerry_string_to_char_buffer(id, (jerry_char_t *)module_name, module_name_sz);
  module_name[module_name_sz] = '\0';
  jerry_release_value(id);
  /* Find corresponding native module */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, module_name) == 0 && builtin_modules[i].fn != NULL) {
      jerry_value_t res = builtin_modules[i].fn();
      jerryxx_set_property(module, MSTR_EXPORTS, res);
      jerry_release_value(res);
    }
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(process_get_builtin_module_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "builtin_module_name")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, builtin_module_name)
  /* Find a builtin js module, return the module function */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, builtin_module_name) == 0) {
      if (builtin_modules[i].size > 0) { /* has js module */
        jerry_value_t fn = jerry_exec_snapshot(builtin_modules[i].code, builtin_modules[i].size, 0, JERRY_SNAPSHOT_EXEC_ALLOW_STATIC);
        return fn;
      } else if (builtin_modules[i].fn != NULL) { /* has native module */
        jerry_value_t fn = jerry_create_external_function(native_module_wrapper_fn);
        return fn;
      }
    }
  }
  /* return undefined */
  return jerry_create_undefined();
}

static void register_global_process_object() {
  jerry_value_t process = jerry_create_object();
  jerryxx_set_property_string(process, MSTR_ARCH, (char *)system_arch);
  jerryxx_set_property_string(process, MSTR_PLATFORM, (char *)system_platform);
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
  jerryxx_set_property(global, MSTR_PROCESS, process);

  jerry_release_value(process);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                      BASE64 ENCODE/DECODE FUNCTIONS                      */
/*                                                                          */
/****************************************************************************/

static void base64_buffer_free_cb(void *native_p) {
  free(native_p);
}

JERRYXX_FUN(encode_fn) {
  JERRYXX_CHECK_ARG(0, "binaryData")
  jerry_value_t binary_data = JERRYXX_GET_ARG(0);
  size_t encoded_data_sz;
  unsigned char *encoded_data = NULL;
  if (jerry_value_is_array(binary_data)) { /* for Array<number> */
    size_t len = jerry_get_array_length(binary_data);
    uint8_t buf[len];
    for (int i = 0; i < len; i++) {
      jerry_value_t item = jerry_get_property_by_index(binary_data, i);
      if (jerry_value_is_number(item)) {
        buf[i] = (uint8_t) jerry_get_number_value(item);
      } else {
        buf[i] = 0; // write 0 for non-number item.
      }
    }
    encoded_data = base64_encode(buf, len, &encoded_data_sz);
  } else if (jerry_value_is_arraybuffer(binary_data)) { /* for ArrayBuffer */
    size_t len = jerry_get_arraybuffer_byte_length(binary_data);
    uint8_t *buf = jerry_get_arraybuffer_pointer(binary_data);
    encoded_data = base64_encode(buf, len, &encoded_data_sz);
  } else if (jerry_value_is_typedarray(binary_data)) { /* for TypedArrays (Uint8Array, Int16Array, ...) */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(binary_data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t *buf = jerry_get_arraybuffer_pointer(array_buffer);
    encoded_data = base64_encode(buf, len, &encoded_data_sz);
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(binary_data)) { /* for string */
    jerry_size_t len = jerry_get_string_size(binary_data);
    uint8_t buf[len];
    jerry_string_to_char_buffer(binary_data, buf, len);
    encoded_data = base64_encode(buf, len, &encoded_data_sz);
  } else {
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Unsupported binary data.");
  }
  if (encoded_data != NULL && encoded_data_sz > 0) {
    jerry_value_t result = jerry_create_string_sz(encoded_data, encoded_data_sz - 1);
    free(encoded_data);
    return result;
  } else {
    return jerry_create_undefined();
  }
}

JERRYXX_FUN(decode_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "encodedData")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, encoded_data)
  size_t decoded_data_sz;
  unsigned char *decoded_data = base64_decode((unsigned char *) encoded_data,
      encoded_data_sz, &decoded_data_sz);
  if (decoded_data != NULL) {
    jerry_value_t buffer = jerry_create_arraybuffer_external(decoded_data_sz, decoded_data, base64_buffer_free_cb);
    return buffer;
  } else {
    return jerry_create_undefined();
  }
}

static void register_global_base64() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_function(global, MSTR_ENCODE, encode_fn);
  jerryxx_set_property_function(global, MSTR_DECODE, decode_fn);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                                ETC FUNCTIONS                             */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(print_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value(JERRYXX_GET_ARG(i));
    }
  }
  return jerry_create_undefined();
}

static void register_global_etc() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_function(global, MSTR_PRINT, print_fn);
  jerry_release_value(global);
}

/******************************************************************************/

static void run_startup_module() {
  jerry_value_t res = jerry_exec_snapshot((const uint32_t *)module_startup_code, module_startup_size, 0, JERRY_SNAPSHOT_EXEC_ALLOW_STATIC);
  jerry_value_t this_val = jerry_create_undefined ();
  jerry_value_t ret_val = jerry_call_function (res, this_val, NULL, 0);
  jerry_release_value (ret_val);
  jerry_release_value (this_val);
  jerry_release_value (res);
}

static void run_board_module() {
  jerry_value_t res = jerry_exec_snapshot((const uint32_t *)module_board_code, module_board_size, 0, JERRY_SNAPSHOT_EXEC_ALLOW_STATIC);
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
  register_global_base64();
  register_global_etc();
  run_startup_module();
  run_board_module();
}
