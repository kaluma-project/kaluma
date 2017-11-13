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

#ifndef __KAMELEON_GPIO_H
#define __KAMELEON_GPIO_H

#include <stdint.h>

typedef enum {
  KAMELEON_GPIO_MODE_INPUT,
  KAMELEON_GPIO_MODE_OUPUT_PP,
  KAMELEON_GPIO_MODE_OUPUT_OD,
  KAMELEON_GPIO_MODE_AF_PP,
  KAMELEON_GPIO_MODE_AF_OD
} kameleon_gpio_mode_t;

#define KAMELEON_GPIO_LOW 0
#define KAMELEON_GPIO_HIGH 1

void kameleon_gpio_pin_mode(uint8_t pin, kameleon_gpio_mode_t mode);
void kameleon_gpio_write(uint8_t pin, uint8_t value);
void kameleon_gpio_toggle(uint8_t pin);
uint8_t kameleon_gpio_read(uint8_t pin);

#endif /* __KAMELEON_GPIO_H */
