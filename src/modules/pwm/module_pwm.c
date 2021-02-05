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

#include <stdlib.h>
#include "jerryscript.h"
#include "jerryxx.h"
#include "pwm_magic_strings.h"
#include "pwm.h"

JERRYXX_FUN(pwm_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "frequency");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "duty");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(1, PWM_DEFAULT_FREQUENCY);
  double duty = JERRYXX_GET_ARG_NUMBER_OPT(2, PWM_DEFAULT_DUTY);
  if (duty < PWM_DUTY_MIN)
    duty = PWM_DUTY_MIN;
  else if (duty > PWM_DUTY_MAX)
    duty = PWM_DUTY_MAX;
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PWM_PIN, pin);
  if (pwm_setup(pin, frequency, duty) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  } else {
    return jerry_create_undefined();
  }
}

JERRYXX_FUN(pwm_start_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  if (pwm_start(pin) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_stop_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  if (pwm_stop(pin) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_get_frequency_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  double frequency = pwm_get_frequency(pin);
  if (frequency == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_number(frequency);
}

JERRYXX_FUN(pwm_set_frequency_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "frequency");
  double frequency = JERRYXX_GET_ARG_NUMBER(0);

  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  if (pwm_set_frequency(pin, frequency) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_get_duty_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  double duty = pwm_get_duty(pin);
  if (duty == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_number(duty);
}

JERRYXX_FUN(pwm_set_duty_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "duty");
  double duty = JERRYXX_GET_ARG_NUMBER(0);
  if (duty < PWM_DUTY_MIN)
    duty = PWM_DUTY_MIN;
  else if (duty > PWM_DUTY_MAX)
    duty = PWM_DUTY_MAX;

  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  if (pwm_set_duty(pin, duty) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_close_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  jerry_release_value(pin_value);

  if (pwm_close(pin) == PWMPORT_ERROR) {
    char errmsg[255];
    sprintf(errmsg, "The pin \"%d\" can't be used for PWM", pin);
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) errmsg);
  }
  return jerry_create_undefined();
}

jerry_value_t module_pwm_init() {
  /* PWM class */
  jerry_value_t pwm_ctor = jerry_create_external_function(pwm_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property(pwm_ctor, "prototype", prototype);
  jerry_release_value (prototype);
  jerryxx_set_property_function(prototype, MSTR_PWM_START, pwm_start_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_STOP, pwm_stop_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_GET_FREQUENCY, pwm_get_frequency_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_SET_FREQUENCY, pwm_set_frequency_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_GET_DUTY, pwm_get_duty_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_SET_DUTY, pwm_set_duty_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_CLOSE, pwm_close_fn);

  /* pwm module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_PWM_PWM, pwm_ctor);
  jerry_release_value (pwm_ctor);

  return exports;
}
