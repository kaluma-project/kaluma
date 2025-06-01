/* Copyright (c) 2017 Kaluma
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

#include "global.h"

#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "base64.h"
#include "board.h"
#include "err.h"
#include "gpio.h"
#include "io.h"
// #include "jerryscript-ext/handler.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "kaluma_config.h"
#include "kaluma_modules.h"
#include "magic_strings.h"
#include "pwm.h"
#include "repl.h"
#include "runtime.h"
#include "system.h"
#include "tty.h"
#include "utils.h"

static void register_global_objects() {
  jerry_value_t global_object = jerry_current_realm();
  jerry_value_t prop_name =
      jerry_string_sz((const char *)MSTR_GLOBAL);
  jerry_object_set(global_object, prop_name, global_object);
  jerry_value_free(prop_name);
  jerry_value_free(global_object);
}

/****************************************************************************/
/*                                                                          */
/*                          DIGITAL I/O FUNCTIONS                           */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(pin_mode_fn) {
  JERRYXX_CHECK_ARG(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "mode");
  jerry_value_t pin = JERRYXX_GET_ARG(0);
  km_gpio_io_mode_t mode =
      (km_gpio_io_mode_t)JERRYXX_GET_ARG_NUMBER_OPT(1, KM_GPIO_IO_MODE_INPUT);
  if (jerry_value_is_number(pin)) {
    uint8_t pin_num = jerry_value_as_number(pin);
    int ret = km_gpio_set_io_mode(pin_num, mode);
    if (ret < 0) {
      return jerry_exception_value(create_system_error(ret), true);
    }
  } else if (jerry_value_is_array(pin)) {
    int pin_len = jerry_array_length(pin);
    for (int i = 0; i < pin_len; i++) {
      jerry_value_t item = jerry_object_get_index(pin, i);
      if (jerry_value_is_number(item)) {
        uint8_t p = jerry_value_as_number(item);
        int ret = km_gpio_set_io_mode(p, mode);
        if (ret < 0) {
          return jerry_exception_value(create_system_error(ret), true);
        }
      } else {
        return jerry_error_sz(
            JERRY_ERROR_TYPE,
            "\"pin\" argument must be a number or number[]");
      }
      jerry_value_free(item);
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "\"pin\" argument must be a number or number[]");
  }
  return jerry_undefined();
}

JERRYXX_FUN(digital_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  int value = km_gpio_read(pin);
  if (value < 0) {
    return jerry_exception_value(create_system_error(value), true);
  }
  return jerry_number(value);
}

JERRYXX_FUN(digital_write_fn) {
  JERRYXX_CHECK_ARG(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  jerry_value_t pin = JERRYXX_GET_ARG(0);
  uint32_t value = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(1, KM_GPIO_LOW);
  if (jerry_value_is_number(pin)) {
    uint8_t pin_num = jerry_value_as_number(pin);
    int ret = km_gpio_write(pin_num, value);
    if (ret < 0) {
      return jerry_exception_value(create_system_error(ret), true);
    }
  } else if (jerry_value_is_array(pin)) {
    int pin_len = jerry_array_length(pin);
    for (int i = 0; i < pin_len; i++) {
      jerry_value_t item = jerry_object_get_index(pin, pin_len - i - 1);
      if (jerry_value_is_number(item)) {
        uint8_t p = jerry_value_as_number(item);
        uint32_t v = ((value >> i) & 0x01);
        int ret = km_gpio_write(p, v);
        if (ret < 0) {
          return jerry_exception_value(create_system_error(ret), true);
        }
      } else {
        return jerry_error_sz(
            JERRY_ERROR_TYPE,
            "\"pin\" argument must be a number or number[]");
      }
      jerry_value_free(item);
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "\"pin\" argument must be a number or number[]");
  }
  return jerry_undefined();
}

JERRYXX_FUN(digital_toggle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  int ret = km_gpio_toggle(pin);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * Timeout check function
 */
uint8_t check_timeout(uint32_t start, uint32_t timeout) {
  uint32_t tout, now;
  now = km_micro_gettime();
  if (now >= start)
    tout = now - start;
  else
    tout = (km_micro_maxtime() - start) + now;
  if (tout > timeout) return 1;
  return 0;
}

/**
 * Read pulse signal on the GPIO pin.
 */
static int pulse_read(uint8_t pin, uint8_t state, uint32_t *arr, size_t length,
                      uint32_t timeout) {
  size_t cnt = 0;
  uint32_t start, last, now;
  int pin_state;
  int pre_pin_state = km_gpio_read(pin);
  start = km_micro_gettime();
  while ((state < 255) && (pre_pin_state != state)) {
    if (check_timeout(start, timeout)) return 0;
    pre_pin_state = km_gpio_read(pin);
  }
  start = km_micro_gettime();
  last = start;
  do {
    pin_state = km_gpio_read(pin);
    now = km_micro_gettime();
    if (pin_state != pre_pin_state) {
      pre_pin_state = pin_state;
      if (now >= last)
        arr[cnt++] = now - last;
      else
        arr[cnt++] = (km_micro_maxtime() - last) + now;
      last = now;
    } else {
      if (check_timeout(start, timeout)) return cnt;
    }
  } while (cnt < length);
  return cnt;
}

/**
 * make pulse on the GPIO pin.
 */
static int pulse_write(uint8_t pin, uint8_t state, uint32_t *arr,
                       size_t length) {
  uint32_t delay;
  int pin_state = (state == KM_GPIO_LOW) ? KM_GPIO_LOW : KM_GPIO_HIGH;
  km_gpio_write(pin, pin_state);
  for (int i = 0; i < length; i++) {
    delay = arr[i];
    km_micro_delay(delay);
    km_gpio_toggle(pin);
  }
  return 0;
}

JERRYXX_FUN(pulse_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER(1, "count");
  JERRYXX_CHECK_ARG_OBJECT_OPT(2, "options");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t count = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint32_t timeout = 1000000U;  // default is 1s
  uint8_t state = 255;          // 255 means undefined.
  uint8_t mode = 255;           // 255 means undefined.
  uint8_t trigger_pin = 255;
  uint8_t trigger_start_state = 0;  // default is LOW
  size_t trigger_len = 0;
  uint32_t *trigger_buf = NULL;
  uint32_t *buf = malloc(count * 4);

  // read options
  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t options = JERRYXX_GET_ARG(2);
    timeout = jerryxx_get_property_number(options, MSTR_TIMEOUT, 1000000U);
    state = jerryxx_get_property_number(options, MSTR_START_STATE, 255);
    mode = jerryxx_get_property_number(options, MSTR_MODE, 255);
    jerry_value_t trigger = jerryxx_get_property(options, MSTR_TRIGGER);
    if (jerry_value_is_object(trigger)) {
      trigger_pin = jerryxx_get_property_number(trigger, MSTR_PIN, pin);
      trigger_start_state =
          jerryxx_get_property_number(trigger, MSTR_START_STATE, 0);
      jerry_value_t trigger_interval =
          jerryxx_get_property(trigger, MSTR_INTERVAL);
      if (jerry_value_is_array(trigger_interval)) {
        trigger_len = jerry_array_length(trigger_interval);
        if (trigger_len > 0) {
          trigger_buf = malloc(trigger_len * 4);
          for (int i = 0; i < trigger_len; i++) {
            jerry_value_t item =
                jerry_object_get_index(trigger_interval, i);
            if (jerry_value_is_number(item)) {
              trigger_buf[i] = (uint32_t)jerry_value_as_number(item);
            } else {
              trigger_buf[i] = 0;
            }
            jerry_value_free(item);
          }
        }
      }
      jerry_value_free(trigger_interval);
    }
    jerry_value_free(trigger);
  }

  // triggering
  if (trigger_pin < 255) {
    km_gpio_set_io_mode(trigger_pin, KM_GPIO_IO_MODE_OUTPUT);
    pulse_write(trigger_pin, trigger_start_state, trigger_buf, trigger_len);
  }

  // set initial mode
  if (mode < 255) km_gpio_set_io_mode(pin, mode);

  // read pulse
  count = pulse_read(pin, state, buf, count, timeout);

  // free trigger buffer
  if (trigger_buf) {
    free(trigger_buf);
  }

  // return pulse data
  if (count) {
    jerry_value_t output_array = jerry_array(count);
    for (int i = 0; i < count; i++) {
      jerry_value_t val = jerry_number(buf[i]);
      jerry_value_free(jerry_object_set_index(output_array, i, val));
      jerry_value_free(val);
    }
    free(buf);
    return output_array;
  }
  free(buf);
  return jerry_null();
}

JERRYXX_FUN(pulse_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER(1, "value");
  JERRYXX_CHECK_ARG(2, "interval")
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  size_t length = 0;
  uint8_t value = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  jerry_value_t interval_arr = JERRYXX_GET_ARG(2);
  if (jerry_value_is_array(interval_arr)) { /* for Array<number> */
    length = jerry_array_length(interval_arr);
    uint32_t buf[length];
    for (int i = 0; i < length; i++) {
      jerry_value_t item = jerry_object_get_index(interval_arr, i);
      if (jerry_value_is_number(item)) {
        if (jerry_value_as_number(item) > 0xFFFFU)
          buf[i] = 0xFFFF;
        else
          buf[i] = (uint32_t)jerry_value_as_number(item);
      } else {
        buf[i] = 0;  // write 0 for non-number item.
      }
      jerry_value_free(item);
    }
    pulse_write(pin, value, buf, length);
  } else {
    char errmsg[255];
    sprintf(errmsg, "The interval is not an array type");
    return jerry_error_sz(JERRY_ERROR_RANGE, (const char *)errmsg);
  }
  return jerry_number(length);
}

static void watch_close_cb(km_io_handle_t *handle) { free(handle); }

static void set_watch_cb(km_io_watch_handle_t *watch) {
  if (jerry_value_is_function(watch->watch_js_cb)) {
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t pin = jerry_number(watch->pin);
    jerry_value_t args[1] = {pin};
    jerry_value_t ret_val =
        jerry_call(watch->watch_js_cb, this_val, args, 1);
    if (jerry_value_is_error(ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
      // clear handle
      jerry_value_free(watch->watch_js_cb);
      km_io_watch_stop(watch);
      km_io_handle_close((km_io_handle_t *)watch, watch_close_cb);
    }
    jerry_value_free(ret_val);
    jerry_value_free(pin);
    jerry_value_free(this_val);
  }
}

JERRYXX_FUN(set_watch_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "events");
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "debounce");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  km_io_watch_mode_t events =
      JERRYXX_GET_ARG_NUMBER_OPT(2, KM_IO_WATCH_MODE_CHANGE);
  uint32_t debounce = JERRYXX_GET_ARG_NUMBER_OPT(3, 0);
  km_io_watch_handle_t *watch = malloc(sizeof(km_io_watch_handle_t));
  km_io_watch_init(watch);
  watch->watch_js_cb = jerry_value_copy(callback);
  km_io_watch_start(watch, set_watch_cb, pin, events, debounce);
  return jerry_number(watch->base.id);
}

JERRYXX_FUN(clear_watch_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "id");
  int id = (int)JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  km_io_watch_handle_t *watch = km_io_watch_get_by_id(id);
  if (watch != NULL) {
    jerry_value_free(watch->watch_js_cb);
    km_io_watch_stop(watch);
    km_io_handle_close((km_io_handle_t *)watch, watch_close_cb);
  }
  return jerry_undefined();
}

static void register_global_digital_io() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_number(global, MSTR_HIGH, KM_GPIO_HIGH);
  jerryxx_set_property_number(global, MSTR_LOW, KM_GPIO_LOW);
  jerryxx_set_property_number(global, MSTR_INPUT,
                              (double)KM_GPIO_IO_MODE_INPUT);
  jerryxx_set_property_number(global, MSTR_OUTPUT,
                              (double)KM_GPIO_IO_MODE_OUTPUT);
  jerryxx_set_property_number(global, MSTR_INPUT_PULLUP,
                              (double)KM_GPIO_IO_MODE_INPUT_PULLUP);
  jerryxx_set_property_number(global, MSTR_INPUT_PULLDOWN,
                              (double)KM_GPIO_IO_MODE_INPUT_PULLDOWN);
  jerryxx_set_property_number(global, MSTR_LOW_LEVEL,
                              (double)KM_IO_WATCH_MODE_LOW_LEVEL);
  jerryxx_set_property_number(global, MSTR_HIGH_LEVEL,
                              (double)KM_IO_WATCH_MODE_HIGH_LEVEL);
  jerryxx_set_property_number(global, MSTR_RISING,
                              (double)KM_IO_WATCH_MODE_RISING);
  jerryxx_set_property_number(global, MSTR_FALLING,
                              (double)KM_IO_WATCH_MODE_FALLING);
  jerryxx_set_property_number(global, MSTR_CHANGE,
                              (double)KM_IO_WATCH_MODE_CHANGE);
  jerryxx_set_property_function(global, MSTR_PIN_MODE, pin_mode_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_READ, digital_read_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_WRITE, digital_write_fn);
  jerryxx_set_property_function(global, MSTR_DIGITAL_TOGGLE, digital_toggle_fn);
  jerryxx_set_property_function(global, MSTR_PULSE_READ, pulse_read_fn);
  jerryxx_set_property_function(global, MSTR_PULSE_WRITE, pulse_write_fn);
  jerryxx_set_property_function(global, MSTR_SET_WATCH, set_watch_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_WATCH, clear_watch_fn);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                           INTERRUPT FUNCTIONS                            */
/*                                                                          */
/****************************************************************************/

static jerry_value_t irq_js_cb[GPIO_MAX];

static void irq_cb(uint8_t pin, km_gpio_io_mode_t mode) {
  jerry_value_t cb = irq_js_cb[pin];
  if (jerry_value_is_function(cb)) {
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t arg_pin = jerry_number(pin);
    jerry_value_t arg_mode = jerry_number(mode);
    jerry_value_t args_p[2] = {arg_pin, arg_mode};
    jerry_value_t ret_val = jerry_call(cb, this_val, args_p, 2);
    if (jerry_value_is_error(ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
    }
    jerry_value_free(arg_pin);
    jerry_value_free(arg_mode);
    jerry_value_free(ret_val);
    jerry_value_free(this_val);
  }
  jerry_value_free(cb);
}

JERRYXX_FUN(attach_interrupt_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_FUNCTION(1, "callback");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "events");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t callback = JERRYXX_GET_ARG(1);
  km_io_watch_mode_t events =
      JERRYXX_GET_ARG_NUMBER_OPT(2, KM_IO_WATCH_MODE_CHANGE);
  if ((events & KM_IO_WATCH_MODE_CHANGE) == 0) {
    char errmsg[255];
    sprintf(errmsg,
            "Only RISING, FALLING and CHANGE can be set for interrupt event.");
    return jerry_error_sz(JERRY_ERROR_RANGE, (const char *)errmsg);
  }
  if (jerry_value_is_function(callback)) {
    irq_js_cb[pin] = jerry_value_copy(callback);
    km_gpio_irq_set_callback(irq_cb);
  }
  if (km_gpio_irq_attach(pin, events) < 0) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_error_sz(JERRY_ERROR_RANGE, (const char *)errmsg);
  }
  return jerry_undefined();
}

JERRYXX_FUN(detach_interrupt_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  if (km_gpio_irq_detach(pin) < 0) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for GPIO", pin);
    return jerry_error_sz(JERRY_ERROR_RANGE, (const char *)errmsg);
  }
  km_gpio_irq_detach(pin);
  jerry_value_free(irq_js_cb[pin]);
  irq_js_cb[pin] = 0;
  return jerry_undefined();
}

JERRYXX_FUN(enable_interrupts_fn) {
  km_gpio_irq_set_callback(irq_cb);
  km_gpio_irq_enable();
  return jerry_undefined();
}

JERRYXX_FUN(disable_interrupts_fn) {
  km_gpio_irq_set_callback(NULL);
  km_gpio_irq_disable();
  return jerry_undefined();
}

static void register_global_interrupts() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_function(global, MSTR_ATTACH_INTERRUPT,
                                attach_interrupt_fn);
  jerryxx_set_property_function(global, MSTR_DETACH_INTERRUPT,
                                detach_interrupt_fn);
  jerryxx_set_property_function(global, MSTR_ENABLE_INTERRUPTS,
                                enable_interrupts_fn);
  jerryxx_set_property_function(global, MSTR_DISABLE_INTERRUPTS,
                                disable_interrupts_fn);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                              TIMER FUNCTIONS                             */
/*                                                                          */
/****************************************************************************/

static void timer_close_cb(km_io_handle_t *handle) { free(handle); }

static void set_timer_cb(km_io_timer_handle_t *timer) {
  if (jerry_value_is_function(timer->timer_js_cb)) {
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t ret_val =
        jerry_call(timer->timer_js_cb, this_val, NULL, 0);
    if (jerry_value_is_error(ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
      // clear handle
      jerry_value_free(timer->timer_js_cb);
      km_io_timer_stop(timer);
      km_io_handle_close((km_io_handle_t *)timer, timer_close_cb);
    } else {
      if (timer->repeat == false) {
        jerry_value_free(timer->timer_js_cb);
        km_io_timer_stop(timer);
        km_io_handle_close((km_io_handle_t *)timer, timer_close_cb);
      }
    }
    jerry_value_free(ret_val);
    jerry_value_free(this_val);
  }
}

JERRYXX_FUN(set_timeout_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "delay");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint64_t delay = (uint64_t)JERRYXX_GET_ARG_NUMBER(1);
  km_io_timer_handle_t *timer = malloc(sizeof(km_io_timer_handle_t));
  km_io_timer_init(timer);
  timer->timer_js_cb = jerry_value_copy(callback);
  km_io_timer_start(timer, set_timer_cb, delay, false);
  return jerry_number(timer->base.id);
}

JERRYXX_FUN(set_interval_fn) {
  JERRYXX_CHECK_ARG_FUNCTION(0, "callback");
  JERRYXX_CHECK_ARG_NUMBER(1, "delay");
  jerry_value_t callback = JERRYXX_GET_ARG(0);
  uint64_t delay = (uint64_t)JERRYXX_GET_ARG_NUMBER(1);
  km_io_timer_handle_t *timer = malloc(sizeof(km_io_timer_handle_t));
  km_io_timer_init(timer);
  timer->timer_js_cb = jerry_value_copy(callback);
  km_io_timer_start(timer, set_timer_cb, delay, true);
  return jerry_number(timer->base.id);
}

JERRYXX_FUN(clear_timer_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id");
  int id = (int)JERRYXX_GET_ARG_NUMBER(0);
  km_io_timer_handle_t *timer = km_io_timer_get_by_id(id);
  if (timer != NULL) {
    jerry_value_free(timer->timer_js_cb);
    km_io_timer_stop(timer);
    km_io_handle_close((km_io_handle_t *)timer, timer_close_cb);
  }
  return jerry_undefined();
}

JERRYXX_FUN(delay_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "msec");
  uint32_t delay_val = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  km_delay(delay_val);
  return jerry_undefined();
}

JERRYXX_FUN(millis_fn) {
  uint64_t msec = km_gettime();
  return jerry_number(msec);
}

JERRYXX_FUN(delay_microseconds_fn) {
  JERRYXX_CHECK_ARG_NUMBER_OPT(0, "usec");
  uint32_t delay_val = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(0, 0);
  km_micro_delay(delay_val);
  return jerry_undefined();
}

JERRYXX_FUN(micros_fn) {
  uint64_t usec = km_micro_gettime();
  return jerry_number(usec);
}

static void register_global_timers() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_function(global, MSTR_SET_TIMEOUT, set_timeout_fn);
  jerryxx_set_property_function(global, MSTR_SET_INTERVAL, set_interval_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_TIMEOUT, clear_timer_fn);
  jerryxx_set_property_function(global, MSTR_CLEAR_INTERVAL, clear_timer_fn);
  jerryxx_set_property_function(global, MSTR_DELAY, delay_fn);
  jerryxx_set_property_function(global, MSTR_MILLIS, millis_fn);
  jerryxx_set_property_function(global, MSTR_DELAY_MICROSECONDS,
                                delay_microseconds_fn);
  jerryxx_set_property_function(global, MSTR_MICROS, micros_fn);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                           ANALOG I/O FUNCTIONS                           */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(analog_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  int ret = km_adc_setup(pin);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  km_delay(1);  // To prevent issue #55
  double value = km_adc_read((uint8_t)ret);
  return jerry_number(value);
}

JERRYXX_FUN(analog_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "value");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "frequency");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  double value = JERRYXX_GET_ARG_NUMBER_OPT(1, 0.5);
  if (value < KM_PWM_DUTY_MIN)
    value = KM_PWM_DUTY_MIN;
  else if (value > KM_PWM_DUTY_MAX)
    value = KM_PWM_DUTY_MAX;
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(2, 490);  // Default 490Hz
  int ret = km_pwm_setup(pin, frequency, value);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  } else {
    km_pwm_start(pin);
    return jerry_undefined();
  }
}

static void tone_timeout_cb(km_io_timer_handle_t *timer) {
  uint8_t pin = timer->tag;
  km_pwm_stop(pin);
  km_io_timer_stop(timer);
  km_io_handle_close((km_io_handle_t *)timer, timer_close_cb);
}

JERRYXX_FUN(tone_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "frequency");
  JERRYXX_CHECK_ARG_OBJECT_OPT(2, "options");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(1, 261.626);  // C key frequency
  uint32_t duration = 0;
  double duty = 0.5;
  int8_t inversion = -1;
  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t options = JERRYXX_GET_ARG(2);
    duration = (uint32_t)jerryxx_get_property_number(options, MSTR_DURATION, 0);
    duty = (double)jerryxx_get_property_number(options, MSTR_DUTY, 0.5);
    inversion =
        (int8_t)jerryxx_get_property_number(options, MSTR_INVERSION, -1);
  }
  if ((inversion >= 0) && (km_check_pwm_inv_port(pin, inversion) < 0)) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for tone invert pin",
            inversion);
    return jerry_error_sz(JERRY_ERROR_RANGE, (const char *)errmsg);
  }
  if (duty < KM_PWM_DUTY_MIN)
    duty = KM_PWM_DUTY_MIN;
  else if (duty > KM_PWM_DUTY_MAX)
    duty = KM_PWM_DUTY_MAX;
  int ret = km_pwm_setup(pin, frequency, duty);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  } else {
    if (inversion >= 0) {
      if (km_pwm_set_inversion(pin, inversion) < 0) {
        char errmsg[255];
        sprintf(errmsg, "The pin \"%d\" can't be used for inversion pin",
                inversion);
        return jerry_error_sz(JERRY_ERROR_RANGE,
                                  (const char *)errmsg);
      }
    }
    km_pwm_start(pin);
    if (inversion >= 0) {
      km_pwm_start(inversion);
    }
    // setup timer for duration
    if (duration > 0) {
      km_io_timer_handle_t *timer = malloc(sizeof(km_io_timer_handle_t));
      km_io_timer_init(timer);
      timer->tag = pin;
      km_io_timer_start(timer, tone_timeout_cb, duration, false);
    }
    return jerry_undefined();
  }
}

JERRYXX_FUN(no_tone_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  uint8_t pin = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  int ret = km_pwm_stop(pin);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

static void register_global_analog_io() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_function(global, MSTR_ANALOG_READ, analog_read_fn);
  jerryxx_set_property_function(global, MSTR_ANALOG_WRITE, analog_write_fn);
  jerryxx_set_property_function(global, MSTR_TONE, tone_fn);
  jerryxx_set_property_function(global, MSTR_NO_TONE, no_tone_fn);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                              CONSOLE OBJECT                              */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(console_log_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    km_repl_printf("\33[2K\r");  // set column to 0
    km_repl_printf("\33[0m");    // set to normal color
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        km_repl_printf(" ");
      }
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        km_repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        km_repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
    }
    km_repl_println();
  }
  return jerry_undefined();
}

JERRYXX_FUN(console_error_fn) {
  if (JERRYXX_GET_ARG_COUNT > 0) {
    km_repl_printf("\33[2K\r");  // set column to 0
    km_repl_printf("\33[0m");    // set to normal color
    for (int i = 0; i < JERRYXX_GET_ARG_COUNT; i++) {
      if (i > 0) {
        km_repl_printf(" ");
      }
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        km_repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        km_repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
    }
    km_repl_println();
  }
  return jerry_undefined();
}

static void register_global_console_object() {
  jerry_value_t console = jerry_object();
  jerryxx_set_property_function(console, MSTR_LOG, console_log_fn);
  jerryxx_set_property_function(console, MSTR_ERROR, console_error_fn);
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property(global, MSTR_CONSOLE, console);
  jerry_value_free(console);
  jerry_value_free(global);
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
    if (strcmp(builtin_modules[i].name, native_module_name) == 0 &&
        builtin_modules[i].fn != NULL) {
      return builtin_modules[i].fn();
    }
  }
  /* If no corresponding module, return undefined */
  return jerry_undefined();
}

JERRYXX_FUN(native_module_wrapper_fn) {
  // jerry_value_t exports = JERRYXX_GET_ARG(0); //comment out because it's not
  // used jerry_value_t require = JERRYXX_GET_ARG(1); //comment out because it's
  // not used
  jerry_value_t module = JERRYXX_GET_ARG(2);
  /* Get module name by module.id */
  jerry_value_t id = jerryxx_get_property(module, MSTR_ID);
  jerry_size_t module_name_sz = jerry_string_size(id, JERRY_ENCODING_CESU8);
  char module_name[module_name_sz + 1];
  jerry_string_to_buffer(id, JERRY_ENCODING_CESU8, (jerry_char_t *)module_name, module_name_sz);
  module_name[module_name_sz] = '\0';
  jerry_value_free(id);
  /* Find corresponding native module */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, module_name) == 0 &&
        builtin_modules[i].fn != NULL) {
      jerry_value_t res = builtin_modules[i].fn();
      jerryxx_set_property(module, MSTR_EXPORTS, res);
      jerry_value_free(res);
    }
  }
  return jerry_undefined();
}

JERRYXX_FUN(process_get_builtin_module_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "builtin_module_name")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, builtin_module_name)
  /* Find a builtin js module, return the module function */
  for (int i = 0; i < builtin_modules_length; i++) {
    if (strcmp(builtin_modules[i].name, builtin_module_name) == 0) {
      if (builtin_modules[i].size > 0) { /* has js module */
        jerry_value_t fn = jerry_exec_snapshot(
            builtin_modules[i].code, builtin_modules[i].size, 0,
            JERRY_SNAPSHOT_EXEC_ALLOW_STATIC, NULL);
        return fn;
      } else if (builtin_modules[i].fn != NULL) { /* has native module */
        jerry_value_t fn =
            jerry_function_external(native_module_wrapper_fn);
        return fn;
      }
    }
  }
  return jerry_undefined();
}

JERRYXX_FUN(process_memory_usage_fn) {
  jerry_heap_stats_t stats = {0};
  bool stats_ret = jerry_heap_stats(&stats);
  if (stats_ret) {
    jerry_value_t obj = jerry_object();
    jerryxx_set_property_number(obj, "heapTotal", stats.size);
    jerryxx_set_property_number(obj, "heapUsed", stats.allocated_bytes);
    jerryxx_set_property_number(obj, "heapPeak", stats.peak_allocated_bytes);
    return obj;
  }
  return jerry_undefined();
}

// process.stdin getter
JERRYXX_FUN(process_stdin_getter_fn) {
  jerry_value_t stream = jerryxx_call_require("stream");
  // use stream.__stdin for singleton
  jerry_value_t __stdin = jerryxx_get_property(stream, "__stdin");
  if (jerry_value_is_undefined(__stdin)) {
    jerry_value_free(__stdin);
    jerry_value_t stdin_ctor = jerryxx_get_property(stream, "StdIn");
    jerry_value_t stdin_obj = jerry_construct(stdin_ctor, NULL, 0);
    jerryxx_set_property(stream, "__stdin", stdin_obj);
    jerry_value_free(stdin_ctor);
    jerry_value_free(stream);
    return stdin_obj;
  } else {
    jerry_value_free(stream);
    return __stdin;
  }
}

// process.stdout getter
JERRYXX_FUN(process_stdout_getter_fn) {
  jerry_value_t stream = jerryxx_call_require("stream");
  // use stream.__stdout for singleton
  jerry_value_t __stdout = jerryxx_get_property(stream, "__stdout");
  if (jerry_value_is_undefined(__stdout)) {
    jerry_value_free(__stdout);
    jerry_value_t stdout_ctor = jerryxx_get_property(stream, "StdOut");
    jerry_value_t stdout_obj = jerry_construct(stdout_ctor, NULL, 0);
    jerryxx_set_property(stream, "__stdout", stdout_obj);
    jerry_value_free(stdout_ctor);
    jerry_value_free(stream);
    return stdout_obj;
  } else {
    jerry_value_free(stream);
    return __stdout;
  }
}

static void register_global_process_object() {
  jerry_value_t process = jerry_object();
  jerryxx_set_property_string(process, MSTR_ARCH, KALUMA_SYSTEM_ARCH);
  jerryxx_set_property_string(process, MSTR_PLATFORM, KALUMA_SYSTEM_PLATFORM);
  jerryxx_set_property_string(process, MSTR_VERSION, KALUMA_VERSION);
  jerryxx_set_property_function(process, MSTR_MEMORY_USAGE,
                                process_memory_usage_fn);

  // add `process.binding` function and it's properties
  jerry_value_t binding_fn = jerry_function_external(process_binding_fn);
  jerry_value_t binding_prop =
      jerry_string_sz((const char *)MSTR_BINDING);
  jerry_object_set(process, binding_prop, binding_fn);
  jerry_value_free(binding_prop);
  for (int i = 0; i < builtin_modules_length; i++) {
    if (builtin_modules[i].fn != NULL) {
      jerry_value_t value =
          jerry_string_sz((const char *)builtin_modules[i].name);
      jerry_value_t ret = jerry_object_set(binding_fn, value, value);
      jerry_value_free(ret);
      jerry_value_free(value);
    }
  }
  jerry_value_free(binding_fn);

  // add `process.buildin_modules` array property
  jerry_value_t array_modules = jerry_array(builtin_modules_length);
  for (int i = 0; i < builtin_modules_length; i++) {
    jerry_value_t value =
        jerry_string_sz((const char *)builtin_modules[i].name);
    jerry_value_t ret = jerry_object_set_index(array_modules, i, value);
    jerry_value_free(ret);
    jerry_value_free(value);
  }
  jerry_value_t prop_buildin_modules =
      jerry_string_sz((const char *)MSTR_BUILTIN_MODULES);
  jerry_object_set(process, prop_buildin_modules, array_modules);
  jerry_value_free(prop_buildin_modules);
  jerry_value_free(array_modules);

  // add `process.getBuiltinModule` function
  jerryxx_set_property_function(process, MSTR_GET_BUILTIN_MODULE,
                                process_get_builtin_module_fn);

  // add stdin and stdout readonly properties
  jerryxx_define_own_property(process, MSTR_STDIN, process_stdin_getter_fn,
                              NULL);
  jerryxx_define_own_property(process, MSTR_STDOUT, process_stdout_getter_fn,
                              NULL);

  // register 'process' object to global
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property(global, MSTR_PROCESS, process);

  jerry_value_free(process);
  jerry_value_free(global);
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
  return jerry_undefined();
}

/**
 * TextEncoder.prototype.encode() function
 */
JERRYXX_FUN(textencoder_encode_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "input")
  jerry_value_t input = JERRYXX_GET_ARG(0);
  jerry_value_t encoding =
      jerryxx_get_property(JERRYXX_GET_THIS, MSTR_ENCODING);
  jerry_size_t sz = jerry_string_size(encoding, JERRY_ENCODING_CESU8);
  jerry_char_t buf[sz + 1];
  jerry_size_t len = jerry_string_to_buffer(encoding, JERRY_ENCODING_CESU8, buf, sz);
  buf[len] = '\0';
  jerry_value_free(encoding);
  if (strcmp((char *)buf, "ascii") == 0) {
    jerry_size_t len = jerryxx_get_ascii_string_size(input);
    jerry_value_t array = jerry_typedarray(JERRY_TYPEDARRAY_UINT8, len);
    jerry_length_t byteOffset = 0;
    jerry_length_t byteLength = 0;
    jerry_value_t buffer =
        jerry_typedarray_buffer(array, &byteOffset, &byteLength);
    uint8_t *buf = jerry_arraybuffer_data(buffer);
    jerryxx_string_to_ascii_char_buffer(input, buf, len);
    jerry_value_free(buffer);
    return array;
  } else if (strcmp((char *)buf, "utf-8") == 0) {
    jerry_size_t len = jerry_string_size(input, JERRY_ENCODING_UTF8);
    jerry_value_t array = jerry_typedarray(JERRY_TYPEDARRAY_UINT8, len);
    jerry_length_t byteOffset = 0;
    jerry_length_t byteLength = 0;
    jerry_value_t buffer =
        jerry_typedarray_buffer(array, &byteOffset, &byteLength);
    uint8_t *buf = jerry_arraybuffer_data(buffer);
    jerry_string_to_buffer(input, JERRY_ENCODING_UTF8, buf, len);
    jerry_value_free(buffer);
    return array;
  } else {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "Unsupported encoding.");
  }
}

static void register_global_text_encoder() {
  /* TextEncoder class */
  jerry_value_t textencoder_ctor =
      jerry_function_external(textencoder_ctor_fn);
  jerry_value_t textencoder_prototype = jerry_object();
  jerryxx_set_property(textencoder_ctor, MSTR_PROTOTYPE, textencoder_prototype);
  jerryxx_set_property_function(textencoder_prototype, MSTR_ENCODE,
                                textencoder_encode_fn);
  jerry_value_free(textencoder_prototype);

  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property(global, MSTR_TEXT_ENCODER, textencoder_ctor);
  jerry_value_free(textencoder_ctor);
  jerry_value_free(global);
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
  return jerry_undefined();
}

/**
 * TextDecoder.prototype.decode() function
 */
JERRYXX_FUN(textdecoder_decode_fn) {
  JERRYXX_CHECK_ARG(0, "input")
  jerry_value_t input = JERRYXX_GET_ARG(0);
  if (jerry_value_is_typedarray(input) &&
      jerry_typedarray_type(input) ==
          JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_value_t encoding =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_ENCODING);
    jerry_size_t sz = jerry_string_size(encoding, JERRY_ENCODING_CESU8);
    jerry_char_t buf[sz + 1];
    jerry_size_t len = jerry_string_to_buffer(encoding, JERRY_ENCODING_CESU8, buf, sz);
    jerry_value_free(encoding);
    buf[len] = '\0';
    if (strcmp((char *)buf, "ascii") == 0) {
      // return String.fromCharCode.apply(null, input);
      jerry_value_t global = jerry_current_realm();
      jerry_value_t string_class = jerryxx_get_property(global, "String");
      jerry_value_t from_char_code =
          jerryxx_get_property(string_class, "fromCharCode");
      jerry_value_t apply = jerryxx_get_property(from_char_code, "apply");
      jerry_value_t null_value = jerry_null();
      jerry_value_t args[2] = {null_value, input};
      jerry_value_t ret = jerry_call(apply, from_char_code, args, 2);
      jerry_value_free(null_value);
      jerry_value_free(apply);
      jerry_value_free(from_char_code);
      jerry_value_free(string_class);
      jerry_value_free(global);
      return ret;
    } else if (strcmp((char *)buf, "utf-8") == 0) {
      jerry_length_t byteOffset = 0;
      jerry_length_t byteLength = 0;
      jerry_value_t buffer =
          jerry_typedarray_buffer(input, &byteOffset, &byteLength);
      uint8_t *buf = jerry_arraybuffer_data(buffer);
      jerry_value_t str = jerry_string(buf + byteOffset, byteLength, JERRY_ENCODING_UTF8);
      jerry_value_free(buffer);
      return str;
    } else {
      return jerry_error_sz(JERRY_ERROR_COMMON,
                                "Unsupported encoding.");
    }
  } else {
    return jerry_error_sz(
        JERRY_ERROR_COMMON, "input must be Uint8Array.");
  }
}

static void register_global_text_decoder() {
  /* TextDecoder class */
  jerry_value_t textdecoder_ctor =
      jerry_function_external(textdecoder_ctor_fn);
  jerry_value_t textdecoder_prototype = jerry_object();
  jerryxx_set_property(textdecoder_ctor, MSTR_PROTOTYPE, textdecoder_prototype);
  jerryxx_set_property_function(textdecoder_prototype, MSTR_DECODE,
                                textdecoder_decode_fn);
  jerry_value_free(textdecoder_prototype);

  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property(global, MSTR_TEXT_DECODER, textdecoder_ctor);
  jerry_value_free(textdecoder_ctor);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                             BTOA/ATOB FUNCTIONS                          */
/*                                                                          */
/****************************************************************************/

JERRYXX_FUN(btoa_fn) {
  JERRYXX_CHECK_ARG(0, "data")
  jerry_value_t binary_data = JERRYXX_GET_ARG(0);
  size_t encoded_data_sz;
  unsigned char *encoded_data = NULL;
  if (jerry_value_is_typedarray(binary_data) &&
      jerry_typedarray_type(binary_data) ==
          JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(binary_data, &byteOffset, &byteLength);
    uint8_t *buf = jerry_arraybuffer_data(array_buffer);
    encoded_data = km_base64_encode(buf + byteOffset, byteLength, &encoded_data_sz);
    jerry_value_free(array_buffer);
  } else if (jerry_value_is_string(binary_data)) { /* for string */
    jerry_size_t len = jerryxx_get_ascii_string_size(binary_data);
    uint8_t buf[len];
    jerryxx_string_to_ascii_char_buffer(binary_data, buf, len);
    encoded_data = km_base64_encode(buf, len, &encoded_data_sz);
  } else {
    return jerry_error_sz(JERRY_ERROR_TYPE,
                              "Unsupported binary data.");
  }
  if (encoded_data != NULL && encoded_data_sz > 0) {
    jerry_value_t result =
        jerry_string(encoded_data, encoded_data_sz - 1, JERRY_ENCODING_CESU8);
    free(encoded_data);
    return result;
  } else {
    return jerry_undefined();
  }
}

JERRYXX_FUN(atob_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "encodedData")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, encoded_data)
  size_t decoded_data_sz;
  unsigned char *decoded_data = km_base64_decode(
      (unsigned char *)encoded_data, encoded_data_sz, &decoded_data_sz);
  if (decoded_data != NULL) {
    jerry_value_t buffer = jerry_arraybuffer_external(
      decoded_data, decoded_data_sz, NULL);
    jerry_value_t array =
        jerry_typedarray_with_buffer(JERRY_TYPEDARRAY_UINT8, buffer);
    jerry_value_free(buffer);
    return array;
  } else {
    return jerry_undefined();
  }
}

static bool is_uri_char(char ch) {
  return ((ch >= 0x30 && ch <= 0x39) ||  // numeric
          (ch >= 0x41 && ch <= 0x5A) ||  // alpha (upper)
          (ch >= 0x61 && ch <= 0x7A) ||  // alpha (lower)
          (ch == '-') || (ch == '_') || (ch == '.') || (ch == '!') ||
          (ch == '~') || (ch == '*') || (ch == '\'') || (ch == '(') ||
          (ch == ')'));
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
  char encoded[size + 1];
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
  return jerry_string_sz(encoded);
}

JERRYXX_FUN(decode_uri_component_fn) {
  JERRYXX_CHECK_ARG(0, "data")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, data)
  char decoded[data_sz];
  int i = 0, p = 0;
  while (i < data_sz) {
    char ch = data[i];
    if (ch == '%') {
      uint8_t bin = km_hex1(data[i + 1]) << 4 | km_hex1(data[i + 2]);
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
  return jerry_string_sz(decoded);
}

static void register_global_encoders() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_function(global, MSTR_BTOA, btoa_fn);
  jerryxx_set_property_function(global, MSTR_ATOB, atob_fn);
  jerryxx_set_property_function(global, MSTR_ENCODE_URI_COMPONENT,
                                encode_uri_component_fn);
  jerryxx_set_property_function(global, MSTR_DECODE_URI_COMPONENT,
                                decode_uri_component_fn);
  jerry_value_free(global);
}

/****************************************************************************/
/*                                                                          */
/*                                SYSTEM ERROR                              */
/*                                                                          */
/****************************************************************************/

/**
 * SystemError() constructor
 */
JERRYXX_FUN(system_error_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "errno")
  int errno = JERRYXX_GET_ARG_NUMBER(0);
  if (errno > 0) errno = -errno;
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_ERRNO, errno);
  jerryxx_set_property_string(JERRYXX_GET_THIS, MSTR_MESSAGE,
                              (char *)errmsg[-errno]);
  return jerry_undefined();
}

static void register_global_system_error() {
  jerry_value_t global = jerry_current_realm();
  /* SystemError class */
  jerry_value_t system_error_ctor =
      jerry_function_external(system_error_ctor_fn);
  jerry_value_t global_error = jerryxx_get_property(global, MSTR_ERROR_CLASS);
  // SystemError extends Error
  jerryxx_inherit(global_error, system_error_ctor);
  // global.SystemError = SystemError
  jerryxx_set_property(global, MSTR_SYSTEM_ERROR, system_error_ctor);
  jerry_value_free(system_error_ctor);
  jerry_value_free(global_error);
  jerry_value_free(global);
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
        km_repl_printf(" ");
      }
      if (jerry_value_is_string(JERRYXX_GET_ARG(i))) {
        km_repl_print_value(JERRYXX_GET_ARG(i));
      } else {
        km_repl_pretty_print(0, 2, JERRYXX_GET_ARG(i));
      }
    }
  }
  return jerry_undefined();
}

JERRYXX_FUN(seed_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "seed")
  uint32_t seed = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  srand(seed);
  return jerry_undefined();
}

static void register_global_etc() {
  jerry_value_t global = jerry_current_realm();
  jerryxx_set_property_function(global, MSTR_PRINT, print_fn);
  jerryxx_set_property_function(global, MSTR_SEED, seed_fn);
  jerry_value_free(global);
}

/******************************************************************************/

static void run_startup_module() {
  jerry_value_t res = jerry_exec_snapshot((const uint32_t *)module_startup_code,
                                          module_startup_size, 0,
                                          JERRY_SNAPSHOT_EXEC_ALLOW_STATIC, NULL);
  jerry_value_t this_val = jerry_undefined();
  jerry_value_t ret_val = jerry_call(res, this_val, NULL, 0);
  if (jerry_value_is_error(ret_val)) {
    // print error
    jerryxx_print_error(ret_val, true);
  }
  jerry_value_free(ret_val);
  jerry_value_free(this_val);
  jerry_value_free(res);
}

static void run_board_module() {
  board_init();
  jerry_value_t board_js = jerry_exec_snapshot(
      (const uint32_t *)module_board_code, module_board_size, 0,
      JERRY_SNAPSHOT_EXEC_ALLOW_STATIC, NULL);
  jerry_value_t this_val = jerry_undefined();
  jerry_value_t global = jerry_current_realm();
  jerry_value_t require = jerryxx_get_property(global, MSTR_REQUIRE);
  jerry_value_t exports = jerry_object();
  jerry_value_t module = jerry_object();
  jerry_value_t args[3] = {exports, require, module};
  jerry_value_t ret_val = jerry_call(board_js, this_val, args, 3);
  if (jerry_value_is_error(ret_val)) {
    // print error
    jerryxx_print_error(ret_val, true);
  }
  jerry_value_t board = jerryxx_get_property(global, MSTR_BOARD);
  jerryxx_set_property_string(board, MSTR_UID, km_getuid());
  jerry_value_free(board);
  jerry_value_free(ret_val);
  jerry_value_free(module);
  jerry_value_free(exports);
  jerry_value_free(require);
  jerry_value_free(global);
  jerry_value_free(this_val);
  jerry_value_free(board_js);
}

void km_global_init() {
  register_global_objects();
  register_global_digital_io();
  register_global_interrupts();
  register_global_analog_io();
  register_global_timers();
  register_global_console_object();
  register_global_process_object();
  register_global_text_encoder();
  register_global_text_decoder();
  register_global_encoders();
  register_global_system_error();
  register_global_etc();
  run_startup_module();
  run_board_module();
}
