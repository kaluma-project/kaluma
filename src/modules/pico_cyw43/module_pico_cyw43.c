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

#include <stdlib.h>

#include "err.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pico/cyw43_arch.h"
#include "pico_cyw43_magic_strings.h"

#define MAX_GPIO_NUM 2

JERRYXX_FUN(pico_cyw43_ctor_fn) {
  cyw43_arch_deinit();
  return jerry_create_undefined();
}

/**
 * PICO_CYW43.prototype.close() function
 */
JERRYXX_FUN(pico_cyw43_close_fn) {
  int ret = cyw43_arch_init();
  if (ret) {
    return jerry_create_error_from_value(create_system_error(ret), true);
  }
  return jerry_create_undefined();
}

static int __check_gpio(uint32_t pin) {
  if (pin > MAX_GPIO_NUM) {
    return -1;
  }
  return 0;
}

JERRYXX_FUN(pico_cyw43_get_gpio) {
  JERRYXX_CHECK_ARG_NUMBER(0, "gpio");
  uint32_t gpio = JERRYXX_GET_ARG_NUMBER(0);
  if (__check_gpio(gpio) < 0) {
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)"GPIO pin is not exist");
  }
  bool ret = cyw43_arch_gpio_get(gpio);
  return jerry_create_boolean(ret);
}

JERRYXX_FUN(pico_cyw43_put_gpio) {
  JERRYXX_CHECK_ARG_NUMBER(0, "gpio");
  JERRYXX_CHECK_ARG_BOOLEAN(1, "value");
  uint32_t gpio = JERRYXX_GET_ARG_NUMBER(0);
  bool value = JERRYXX_GET_ARG_BOOLEAN(1);
  if (__check_gpio(gpio) < 0) {
    return jerry_create_error(JERRY_ERROR_TYPE,
                              (const jerry_char_t *)"GPIO pin is not exist");
  }
  cyw43_arch_gpio_put(gpio, value);
  return jerry_create_undefined();
}
jerry_value_t module_pico_cyw43_init() {
  /* PICO_CYW43 class */
  jerry_value_t pico_cyw43_ctor =
      jerry_create_external_function(pico_cyw43_ctor_fn);
  jerry_value_t prototype = jerry_create_object();
  jerryxx_set_property(pico_cyw43_ctor, "prototype", prototype);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_CLOSE,
                                pico_cyw43_close_fn);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_GETGPIO,
                                pico_cyw43_get_gpio);
  jerryxx_set_property_function(prototype, MSTR_PICO_CYW43_PUTGPIO,
                                pico_cyw43_put_gpio);
  jerry_release_value(prototype);

  /* pico_cyw43 module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_PICO_CYW43_PICO_CYW43, pico_cyw43_ctor);
  jerry_release_value(pico_cyw43_ctor);

  return exports;
}
