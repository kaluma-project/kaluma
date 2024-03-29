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

void km_gpio_init() {}

void km_gpio_cleanup() {}

int km_gpio_set_io_mode(uint8_t pin, km_gpio_io_mode_t mode) { return 0; }

int km_gpio_write(uint8_t pin, uint8_t value) { return 0; }

int km_gpio_read(uint8_t pin) { return 0; }

int km_gpio_toggle(uint8_t pin) { return 0; }

void km_gpio_irq_set_callback(km_gpio_irq_callback_t cb) {}

int km_gpio_irq_attach(uint8_t pin, uint8_t events) { return 0; }

int km_gpio_irq_detach(uint8_t pin) { return 0; }

void km_gpio_irq_enable() {}

void km_gpio_irq_disable() {}
