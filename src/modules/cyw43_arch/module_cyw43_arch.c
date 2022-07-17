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

#include "module_cyw43_arch.h"

#include <stdlib.h>

#include "jerryscript.h"
#include "jerryxx.h"
#include "cyw43_arch.h"
#include "cyw43_arch_magic_strings.h"

/**
 * cyw43_arch.gpioPut(t)
 * args:
 *   pin: {number}
 *   value: {number}
 */
JERRYXX_FUN(cyw43_arch_gpio_put_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "pin")
  JERRYXX_CHECK_ARG_NUMBER(1, "value")
  double pin = JERRYXX_GET_ARG_NUMBER(0);
  double value = JERRYXX_GET_ARG_NUMBER(1);
  km_cyw43_arch_gpio_put((uint8_t)pin, (uint8_t)value);
  return jerry_create_undefined();
}

/**
 * Initialize 'cyw43_arch' module
 */
jerry_value_t module_cyw43_arch_init() {
  /* cyw43_arch module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_CYW43_ARCH_GPIO_PUT, cyw43_arch_gpio_put_fn);


  jerry_value_t global = jerry_get_global_object();
  jerryxx_set_property_number(global, MSTR_CYW43_ARCH_WL_GPIO_LED_PIN, KM_CYW43_ARCH_WL_GPIO_LED_PIN);
  return exports;
}
