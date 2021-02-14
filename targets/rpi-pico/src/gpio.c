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

#include <stdint.h>
#include "gpio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

static int __check_gpio(uint8_t pin)
{
  if ((pin <= 28) && !((pin == 23) || (pin == 24))) {
    return 0;
  } else {
    return KM_GPIOPORT_ERROR; // Not a GPIO pins
  }
}

void km_gpio_init() {
  gpio_init_mask(0xFFFFFFFF); // init all the GPIOs
}

void km_gpio_cleanup() {
  km_gpio_init();
}

int km_gpio_set_io_mode(uint8_t pin, km_gpio_io_mode_t mode) {
  if (__check_gpio(pin) < 0) {
    return KM_GPIOPORT_ERROR;
  }
  if (mode == KM_GPIO_IO_MODE_OUTPUT) {
    gpio_set_dir(pin, true); // Set OUTPUT
  } else {
    gpio_set_input_enabled(pin, true); // Set INPUT
    if (mode == KM_GPIO_IO_MODE_INPUT_PULLUP) {
      gpio_pull_up(pin);
    } else if (mode == KM_GPIO_IO_MODE_INPUT_PULLUP) {
      gpio_pull_down(pin);
    }
  }
  return 0;
}

int km_gpio_write(uint8_t pin, uint8_t value) {
  if (__check_gpio(pin) < 0) {
    return KM_GPIOPORT_ERROR;
  }
  gpio_put(pin, value);
  return 0;
}

int km_gpio_read(uint8_t pin) {
  if (__check_gpio(pin) < 0) {
    return KM_GPIOPORT_ERROR;
  }
  return gpio_get(pin);
}

int km_gpio_toggle(uint8_t pin) {
  if (__check_gpio(pin) < 0) {
    return KM_GPIOPORT_ERROR;
  }
  bool out = gpio_get(pin);
  gpio_put(pin, !out);
  return 0;
}
