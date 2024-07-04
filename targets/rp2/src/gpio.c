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

#include "gpio.h"

#include <stdint.h>
#include <stdlib.h>

#include "board.h"
#include "err.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

static int __check_gpio(uint8_t pin) {
  if (pin <= KALUMA_GPIO_COUNT) {
    return 0;
  } else {
    return EINVPIN;
  }
}

void km_gpio_init() {
  for (uint i = 0; i < NUM_BANK0_GPIOS; i++) {
    gpio_init(i);
    gpio_set_pulls(i, false, false);
  }
#ifdef WIFI_EN_GPIO
  gpio_set_dir(WIFI_EN_GPIO, true);  // Set OUTPUT
  gpio_put(WIFI_EN_GPIO, 0);         // WIFI OFF
#endif
}

void km_gpio_cleanup() {
  for (uint gpio = 0; gpio < NUM_BANK0_GPIOS; gpio++) {
    gpio_acknowledge_irq(gpio, 0xF);
    gpio_set_irq_enabled(gpio, 0xF, false);
  }
  km_gpio_irq_disable();
  km_gpio_init();
}

int km_gpio_set_io_mode(uint8_t pin, km_gpio_io_mode_t mode) {
  if (__check_gpio(pin) < 0) {
    return EINVPIN;
  }
  gpio_set_function(pin, GPIO_FUNC_SIO);
  if (mode == KM_GPIO_IO_MODE_OUTPUT) {
    gpio_set_dir(pin, true);  // Set OUTPUT
  } else {
    gpio_set_dir(pin, false);
    if (mode == KM_GPIO_IO_MODE_INPUT_PULLUP) {
      gpio_pull_up(pin);
    } else if (mode == KM_GPIO_IO_MODE_INPUT_PULLDOWN) {
      gpio_pull_down(pin);
    }
    gpio_set_input_enabled(pin, true);  // Set INPUT
  }
  return 0;
}

int km_gpio_write(uint8_t pin, uint8_t value) {
  if (__check_gpio(pin) < 0) {
    return EINVPIN;
  }
  gpio_put(pin, value);
  return 0;
}

int km_gpio_read(uint8_t pin) {
  if (__check_gpio(pin) < 0) {
    return EINVPIN;
  }
  return gpio_get(pin);
}

int km_gpio_toggle(uint8_t pin) {
  if (__check_gpio(pin) < 0) {
    return EINVPIN;
  }
  bool out = gpio_get(pin);
  gpio_put(pin, !out);
  return 0;
}

static km_gpio_irq_callback_t __gpio_irq_cb = NULL;

static void __gpio_irq_callback(uint gpio, uint32_t events) {
  if (__gpio_irq_cb) {
    __gpio_irq_cb((uint8_t)gpio, (km_gpio_io_mode_t)events);
  }
}

void km_gpio_irq_set_callback(km_gpio_irq_callback_t cb) { __gpio_irq_cb = cb; }

int km_gpio_irq_attach(uint8_t pin, uint8_t events) {
  gpio_acknowledge_irq(pin, 0xF);
  gpio_set_irq_enabled_with_callback(pin, (uint32_t)events, true,
                                     __gpio_irq_callback);
  return 0;
}

int km_gpio_irq_detach(uint8_t pin) {
  gpio_set_irq_enabled(pin, 0xF, false);
  return 0;
}

void km_gpio_irq_enable() {
  for (uint gpio = 0; gpio < NUM_BANK0_GPIOS; gpio++) {
    gpio_acknowledge_irq(gpio, 0xF);
  }
  irq_set_enabled(IO_IRQ_BANK0, true);
}

void km_gpio_irq_disable() { irq_set_enabled(IO_IRQ_BANK0, false); }

static const char* km_gpio_function(const uint8_t pin) {
    const char* mode = "????";
    switch(gpio_get_function(pin)){
        case GPIO_FUNC_XIP:  mode = "XIP "; break;
        case GPIO_FUNC_SPI:  mode = "SPI "; break;
        case GPIO_FUNC_UART: mode = "UART"; break;
        case GPIO_FUNC_I2C:  mode = "I2C "; break;
        case GPIO_FUNC_PWM:  mode = "PWM "; break;
        case GPIO_FUNC_SIO:  mode = "SIO "; break;
        case GPIO_FUNC_PIO0: mode = "PIO0"; break;
        case GPIO_FUNC_PIO1: mode = "PIO1"; break;
        case GPIO_FUNC_GPCK: mode = "GPCK"; break;
        case GPIO_FUNC_USB:  mode = "USB "; break;
        case GPIO_FUNC_NULL: mode = "NULL"; break;
    }
    return(mode);
}

static void show_pin_details(const uint8_t pin) {
    const char* pullUp = (gpio_is_pulled_up(pin) ? " up " : gpio_is_pulled_down(pin) ? "down" : "    ");
    const char* direction = (gpio_get_dir(pin) == 1 ? "[out]," : "[in], ");
    const char* value = gpio_get(pin) ? "high," : "low, ";
    printf("GPIO%02d=%s Dir: %s Mode: [%s], Pull: [%s] \n\r", pin, value, direction, km_gpio_function(pin), pullUp);
}

void km_gpio_overview () {
    for (uint8_t index = 0; index <= 22; index++) {
        show_pin_details(index);
    }
    for (uint8_t index = 25; index <= 28; index++) {
        show_pin_details(index);
    }
}

