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
#include "gpio.h"
#include "runtime.h"

static jerry_value_t pin_mode(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 2);
  // ASSERT(jerry_value_is_number(args_p[0]))
  // ASSERT(jerry_value_is_number(args_p[1]))
  uint8_t pin = (uint8_t) jerry_get_number_value(args_p[0]);
  gpio_io_mode_t mode = (gpio_io_mode_t) jerry_get_number_value(args_p[1]);
  gpio_set_io_mode(pin, mode);
  return jerry_create_undefined();
}

static jerry_value_t digital_read(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 1);
  // ASSERT(jerry_value_is_number(args_p[0]))
  uint8_t pin = (uint8_t) jerry_get_number_value(args_p[0]);
  uint8_t value = gpio_read(pin);
  return jerry_create_number(value);
}

static jerry_value_t digital_write(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 2);
  // ASSERT(jerry_value_is_number(args_p[0]))
  // ASSERT(jerry_value_is_number(args_p[1]))
  uint8_t pin = (uint8_t) jerry_get_number_value(args_p[0]);
  uint8_t value = (uint8_t) jerry_get_number_value(args_p[1]);
  gpio_write(pin, value);
  return jerry_create_undefined();
}

static jerry_value_t digital_toggle(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 1);
  // ASSERT(jerry_value_is_number(args_p[0]))
  uint8_t pin = (uint8_t) jerry_get_number_value(args_p[0]);
  gpio_toggle(pin);
  return jerry_create_undefined();
}

jerry_value_t module_gpio_init() {
  jerry_value_t object = jerry_create_object();
  runtime_register_number(object, "HIGH", GPIO_HIGH);
  runtime_register_number(object, "LOW", GPIO_LOW);
  runtime_register_number(object, "INPUT", (double) GPIO_IO_MODE_INPUT);
  runtime_register_number(object, "OUTPUT", (double) GPIO_IO_MODE_OUTPUT);
  runtime_register_function(object, "pinMode", pin_mode);
  runtime_register_function(object, "digitalRead", digital_read);
  runtime_register_function(object, "digitalWrite", digital_write);
  runtime_register_function(object, "digitalToggle", digital_toggle);
  return object;
}
