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
#include "utils.h"
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

/**
 * Timeout check function
*/
uint8_t check_timeout(uint32_t start, uint32_t timeout) {
  uint32_t tout, now;
  now = micro_gettime();
  if (now >= start)
    tout = now - start;
  else
    tout = (micro_maxtime() - start) + now;
  if (tout > timeout)
    return 1;
  return 0;
}

/**
 * Read pulse signal on the GPIO pin.
*/
int pulse_read(uint8_t pin, uint8_t state, uint16_t *arr, uint8_t length, uint32_t timeout) {
  uint8_t cnt = 0;
  uint32_t start, last, now;
  int pin_state;
  int pre_pin_state = gpio_read(pin);
  start = micro_gettime();
  while ((state < 2) && (pre_pin_state != state)) {
    if (check_timeout(start, timeout))
      return 0;
    pre_pin_state = gpio_read(pin);
  }
  start = micro_gettime();
  last = start;
  do {
    pin_state = gpio_read(pin);
    now = micro_gettime();
    if (pin_state != pre_pin_state) {
      pre_pin_state = pin_state;
      if (now >= last)
        arr[cnt++] = now - last;
      else
        arr[cnt++] = (micro_maxtime() - last) + now;
      last = now;
    } else {
      if (check_timeout(start, timeout))
        return cnt;
    }
  } while (cnt < length);
  return cnt;
}

JERRYXX_FUN(pulse_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER(1, "count");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "timeout");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "startState");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  if (gpio_set_io_mode(pin, GPIO_IO_MODE_INPUT) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  uint8_t count;
  if (JERRYXX_GET_ARG_NUMBER(1) > 128)
    count = 128; /* MAX is 128 */
  else
    count = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
  uint32_t timeout = JERRYXX_GET_ARG_NUMBER_OPT(2, 5000000U); /* Default is 5s */
  if (timeout > 40000000U) { /* MAX 40s */
    timeout = 40000000U;
  }
  uint8_t state = (uint8_t) JERRYXX_GET_ARG_NUMBER_OPT(3, 2); /* 2 means undefined. */
  uint16_t *buf = malloc(256); /* MAX is 128 */

  count = pulse_read(pin, state, buf, count, timeout);
  if (count) {
    jerry_value_t output_array = jerry_create_array(count);
    for (int i = 0; i < count; i++) {
      jerry_value_t val = jerry_create_number(buf[i]);
      jerry_release_value(jerry_set_property_by_index(output_array, i, val));
      jerry_release_value(val);
    }
    free(buf);
    return output_array;
  }
  free(buf);
  return jerry_create_null();
}

/**
 * make pulse on the GPIO pin.
*/
int pulse_write(uint8_t pin, uint8_t state, uint16_t *arr, uint8_t length) {
  uint8_t cnt;
  uint16_t delay;
  int pin_state = (state == GPIO_LOW) ? GPIO_LOW : GPIO_HIGH;
  gpio_write(pin, pin_state);
  for (cnt = 0; cnt < length; cnt++) {
    delay = arr[cnt];
    micro_delay(delay);
    gpio_toggle(pin);
  }
  return 0;
}

JERRYXX_FUN(pulse_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER(1, "value");
  JERRYXX_CHECK_ARG(2, "interval")
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  if (gpio_set_io_mode(pin, GPIO_IO_MODE_OUTPUT) == GPIOPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  size_t length = 0;
  uint8_t value = (uint8_t) JERRYXX_GET_ARG_NUMBER(1);
  jerry_value_t intervalArr = JERRYXX_GET_ARG(2);
  if (jerry_value_is_array(intervalArr)) { /* for Array<number> */
    length = jerry_get_array_length(intervalArr);
    uint16_t buf[length];
    for (int i = 0; i < length; i++) {
      jerry_value_t item = jerry_get_property_by_index(intervalArr, i);
      if (jerry_value_is_number(item)) {
        if (jerry_get_number_value(item) > 0xFFFFU)
          buf[i] = 0xFFFF;
        else
          buf[i] = (uint16_t) jerry_get_number_value(item);
      } else {
        buf[i] = 0; // write 0 for non-number item.
      }
    }
    pulse_write(pin, value, buf, length);
  } else {
    char errmsg[255];
    sprintf(errmsg, "The interval is not an array type");
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_number(length);
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
  jerryxx_set_property_function(global, MSTR_PULSE_READ, pulse_read_fn);
  jerryxx_set_property_function(global, MSTR_PULSE_WRITE, pulse_write_fn);
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
    } else {
      if (timer->repeat == false) {
        jerry_release_value(timer->timer_js_cb);
        io_timer_stop(timer);
        io_handle_close((io_handle_t *) timer, timer_close_cb);
      }
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
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
    }
    repl_println();
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(console_error_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[0m"); // set to normal color
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
    }
    repl_println();
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
  jerryxx_set_property_string(process, MSTR_VERSION, KAMELEON_VERSION);

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
/*                                 TEXTENCODER                              */
/*                                                                          */
/****************************************************************************/

/**
 * TextEncoder() constructor
 */
JERRYXX_FUN(textencoder_ctor_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "label")
  if (JERRYXX_HAS_ARG(0)) {
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_ENCODING, JERRYXX_GET_ARG(0));
  } else {
    jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_ENCODING, "utf-8");
  }
  return jerry_create_undefined();
}

/**
 * TextEncoder.prototype.encode() function
 */
JERRYXX_FUN(textencoder_encode_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "input")
  jerry_value_t input = JERRYXX_GET_ARG(0);
  jerry_value_t encoding = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_ENCODING);
  jerry_size_t sz = jerry_get_string_size(encoding);
  jerry_char_t buf[sz + 1];
  jerry_size_t len = jerry_string_to_char_buffer(encoding, buf, sz);
  buf[len] = '\0';
  if (strcmp((char *) buf, "ascii") == 0) {
    jerry_size_t len = jerryxx_get_ascii_string_size(input);
    jerry_value_t array = jerry_create_typedarray(JERRY_TYPEDARRAY_UINT8, len);
    jerry_length_t byteOffset = 0;
    jerry_length_t byteLength = 0;
    jerry_value_t buffer = jerry_get_typedarray_buffer(array, &byteOffset, &byteLength);
    uint8_t *buf = jerry_get_arraybuffer_pointer(buffer);
    jerryxx_string_to_ascii_char_buffer(input, buf, len);
    jerry_release_value(buffer);
    return array;
  } else if (strcmp((char *) buf, "utf-8") == 0) {
    jerry_size_t len = jerry_get_utf8_string_size(input);
    jerry_value_t array = jerry_create_typedarray(JERRY_TYPEDARRAY_UINT8, len);
    jerry_length_t byteOffset = 0;
    jerry_length_t byteLength = 0;
    jerry_value_t buffer = jerry_get_typedarray_buffer(array, &byteOffset, &byteLength);
    uint8_t *buf = jerry_get_arraybuffer_pointer(buffer);
    jerry_string_to_utf8_char_buffer(input, buf, len);
    jerry_release_value(buffer);
    return array;
  } else {
    return jerry_create_error(JERRY_ERROR_COMMON, (const jerry_char_t *) "Unsupported encoding.");
  }
}

static void register_global_textencoder() {
  /* TextEncoder class */
  jerry_value_t textencoder_ctor = jerry_create_external_function(textencoder_ctor_fn);
  jerry_value_t textencoder_prototype = jerry_create_object();
  jerryxx_set_property(textencoder_ctor, MSTR_PROTOTYPE, textencoder_prototype);
  jerryxx_set_property_function(textencoder_prototype, MSTR_ENCODE, textencoder_encode_fn);
  jerry_release_value(textencoder_prototype);

  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property(global, MSTR_TEXTENCODER, textencoder_ctor);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                                 TEXTDECODER                              */
/*                                                                          */
/****************************************************************************/

/**
 * TextDecoder() constructor
 */
JERRYXX_FUN(textdecoder_ctor_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "label")
  if (JERRYXX_HAS_ARG(0)) {
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_ENCODING, JERRYXX_GET_ARG(0));
  } else {
    jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_ENCODING, "utf-8");
  }
  return jerry_create_undefined();
}

/**
 * TextDecoder.prototype.decode() function
 */
JERRYXX_FUN(textdecoder_decode_fn) {
  JERRYXX_CHECK_ARG(0, "input")
  jerry_value_t input = JERRYXX_GET_ARG(0);
  if (jerry_value_is_typedarray(input) && 
      jerry_get_typedarray_type(input) == JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_value_t encoding = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_ENCODING);
    jerry_size_t sz = jerry_get_string_size(encoding);
    jerry_char_t buf[sz + 1];
    jerry_size_t len = jerry_string_to_char_buffer(encoding, buf, sz);
    buf[len] = '\0';
    if (strcmp((char *) buf, "ascii") == 0) {
      // return String.fromCharCode.apply(null, input);
      jerry_value_t global = jerry_get_global_object();
      jerry_value_t string_class = jerryxx_get_property(global, "String");
      jerry_value_t from_char_code = jerryxx_get_property(string_class, "fromCharCode");
      jerry_value_t apply = jerryxx_get_property(from_char_code, "apply");
      jerry_value_t null_value = jerry_create_null();
      jerry_value_t args[2] = {null_value, input};
      jerry_value_t ret = jerry_call_function(apply, from_char_code, args, 2);
      jerry_release_value(null_value);
      jerry_release_value(apply);
      jerry_release_value(from_char_code);
      jerry_release_value(string_class);
      jerry_release_value(global);
      return ret;
    } else if (strcmp((char *) buf, "utf-8") == 0) {
      jerry_length_t byteOffset = 0;
      jerry_length_t byteLength = 0;
      jerry_value_t buffer = jerry_get_typedarray_buffer(input, &byteOffset, &byteLength);
      uint8_t *buf = jerry_get_arraybuffer_pointer(buffer);
      jerry_value_t str = jerry_create_string_sz_from_utf8(buf, byteLength);
      jerry_release_value(buffer);
      return str;
    } else {
      return jerry_create_error(JERRY_ERROR_COMMON, (const jerry_char_t *) "Unsupported encoding.");
    }
  } else {
    return jerry_create_error(JERRY_ERROR_COMMON, (const jerry_char_t *) "input must be Uint8Array.");
  }
}

static void register_global_textdecoder() {
  /* TextDecoder class */
  jerry_value_t textdecoder_ctor = jerry_create_external_function(textdecoder_ctor_fn);
  jerry_value_t textdecoder_prototype = jerry_create_object();
  jerryxx_set_property(textdecoder_ctor, MSTR_PROTOTYPE, textdecoder_prototype);
  jerryxx_set_property_function(textdecoder_prototype, MSTR_DECODE, textdecoder_decode_fn);
  jerry_release_value(textdecoder_prototype);

  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property(global, MSTR_TEXTDECODER, textdecoder_ctor);
  jerry_release_value(global);
}

/****************************************************************************/
/*                                                                          */
/*                             BTOA/ATOB FUNCTIONS                          */
/*                                                                          */
/****************************************************************************/

static void base64_buffer_free_cb(void *native_p) {
  free(native_p);
}

JERRYXX_FUN(btoa_fn) {
  JERRYXX_CHECK_ARG(0, "data")
  jerry_value_t binary_data = JERRYXX_GET_ARG(0);
  size_t encoded_data_sz;
  unsigned char *encoded_data = NULL;
  if (jerry_value_is_typedarray(binary_data) &&
      jerry_get_typedarray_type(binary_data) == JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer = jerry_get_typedarray_buffer(binary_data, &byteOffset, &byteLength);
    size_t len = jerry_get_arraybuffer_byte_length(array_buffer);
    uint8_t *buf = jerry_get_arraybuffer_pointer(array_buffer);
    encoded_data = base64_encode(buf, len, &encoded_data_sz);
    jerry_release_value(array_buffer);
  } else if (jerry_value_is_string(binary_data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(binary_data);
    uint8_t buf[len];
    jerryxx_string_to_ascii_char_buffer(binary_data, buf, len);
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

JERRYXX_FUN(atob_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "encodedData")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, encoded_data)
  size_t decoded_data_sz;
  unsigned char *decoded_data = base64_decode((unsigned char *) encoded_data,
      encoded_data_sz, &decoded_data_sz);
  if (decoded_data != NULL) {
    jerry_value_t buffer = jerry_create_arraybuffer_external(decoded_data_sz, decoded_data, base64_buffer_free_cb);
    jerry_value_t array = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, buffer);
    jerry_release_value(buffer);
    return array;
  } else {
    return jerry_create_undefined();
  }
}

static bool is_uri_char (char ch) {
  return ((ch >= 0x30 && ch <= 0x39) ||  // numeric
         (ch >= 0x41 && ch <= 0x5A) ||  // alpha (upper)
         (ch >= 0x61 && ch <= 0x7A) ||  // alpha (lower)
         (ch == '-') || (ch == '_') || (ch =='.') ||
         (ch == '!') || (ch == '~') || (ch == '*') ||
         (ch == '\'') ||  (ch == '(') ||  (ch == ')'));
}

JERRYXX_FUN(encode_uri_component_fn) {
  JERRYXX_CHECK_ARG(0, "data")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, data)
  int size = 0;
  for (int i = 0; i < data_sz; i++) {
    char ch = data[i];
    if (is_uri_char(ch)) {
      size++;
    } else {
      size += 3;
    }
  }
  unsigned char encoded[size + 1];
  int p = 0;
  const char hex[] = "0123456789ABCDEF";
  for (int i = 0; i < data_sz; i++) {
    char ch = data[i];
    if (is_uri_char(ch)) {
      encoded[p] = ch;
      p++;
    } else {
      encoded[p] = '%';
      encoded[p + 1] = hex[ch >> 4];
      encoded[p + 2] = hex[ch & 0x0F];
      p += 3;
    }
  }
  encoded[size] = '\0';
  return jerry_create_string(encoded);
}

JERRYXX_FUN(decode_uri_component_fn) {
  JERRYXX_CHECK_ARG(0, "data")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, data)
  unsigned char decoded[data_sz];
  int i = 0, p = 0;
  while (i < data_sz) {
    char ch = data[i];
    if (ch == '%') {
      uint8_t bin = hex1(data[i + 1]) << 4 | hex1(data[i + 2]);
      decoded[p] = bin;
      i += 3;
      p++;
    } else {
      decoded[p] = ch;
      i++;
      p++;
    }
  }
  decoded[p] = '\0';
  return jerry_create_string(decoded);
}

static void register_global_encoders() {
  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_function(global, MSTR_BTOA, btoa_fn);
  jerryxx_set_property_function(global, MSTR_ATOB, atob_fn);
  jerryxx_set_property_function(global, MSTR_ENCODE_URI_COMPONENT, encode_uri_component_fn);
  jerryxx_set_property_function(global, MSTR_DECODE_URI_COMPONENT, decode_uri_component_fn);
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
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
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
  register_global_textencoder();
  register_global_textdecoder();
  register_global_encoders();
  register_global_etc();
  run_startup_module();
  run_board_module();
}
