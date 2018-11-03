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
#include "jerryscript.h"
#include "jerryxx.h"
#include "pwm_magic_strings.h"
#include "pwm.h"

#define PWM_DEFAULT_FREQUENCY 490
#define PWM_DEFAULT_DUTY 1.0

JERRYXX_FUN(pwm_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pin");
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "frequency");
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "duty");
  uint8_t pin = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  double frequency = JERRYXX_GET_ARG_NUMBER_OPT(1, PWM_DEFAULT_FREQUENCY);
  double duty = JERRYXX_GET_ARG_NUMBER_OPT(2, PWM_DEFAULT_DUTY);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_PWM_PIN, pin);
  pwm_setup(pin, frequency, duty);
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_start_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  pwm_start(pin);
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_stop_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  pwm_stop(pin);
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_get_frequency_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  double frequency = pwm_get_frequency(pin);
  return jerry_create_number(frequency);
}

JERRYXX_FUN(pwm_set_frequency_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "frequency");
  double frequency = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  pwm_set_frequency(pin, frequency);
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_get_duty_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  double duty = pwm_get_duty(pin);
  return jerry_create_number(duty);
}

JERRYXX_FUN(pwm_set_duty_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "duty");
  double duty = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  pwm_set_duty(pin, duty);
  return jerry_create_undefined();
}

JERRYXX_FUN(pwm_close_fn) {
  jerry_value_t pin_value = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_PWM_PIN);
  if (!jerry_value_is_number(pin_value)) {
    return JERRYXX_CREATE_ERROR("PWM pin is not setup.");
  }
  uint8_t pin = (uint8_t) jerry_get_number_value(pin_value);
  pwm_stop(pin);
  return jerry_create_undefined();
}

jerry_value_t module_pwm_init() {
  /* PWM constructor */
  jerry_value_t ctor = jerry_create_external_function(pwm_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property(ctor, "prototype", prototype);
  jerry_release_value (prototype);
  /* PWM instance properties */
  jerryxx_set_property_function(prototype, MSTR_PWM_START, pwm_start_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_STOP, pwm_stop_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_GET_FREQUENCY, pwm_get_frequency_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_SET_FREQUENCY, pwm_set_frequency_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_GET_DUTY, pwm_get_duty_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_SET_DUTY, pwm_set_duty_fn);
  jerryxx_set_property_function(prototype, MSTR_PWM_CLOSE, pwm_close_fn);
  return ctor;
}
