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

#ifndef __BOARD_H
#define __BOARD_H

#include <stdint.h>

extern const char board_arch[];
extern const char board_platform[];
extern const char board_name[];

extern const uint8_t pin_num;
extern const uint8_t led_num;
extern const uint8_t button_num;
extern const uint8_t pwm_num;
extern const uint8_t adc_num;
extern const uint8_t i2c_num;
extern const uint8_t spi_num;
extern const uint8_t uart_num;
extern const uint8_t led_pins[];
extern const uint8_t button_pins[];
extern const uint8_t pwm_pins[];
extern const uint8_t adc_pins[];

#endif /* __BOARD_H */
