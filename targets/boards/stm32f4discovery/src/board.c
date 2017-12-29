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

#include <stdint.h>

const char board_arch[] = "arm";
const char board_platform[] = "unknown";
const char board_name[] = "stm32f4discovery";

const uint8_t pin_num = 16;
const uint8_t led_num = 1;
const uint8_t switch_num = 1;
const uint8_t pwm_num = 2;
const uint8_t adc_num = 2;
const uint8_t i2c_num = 1;
const uint8_t spi_num = 1;
const uint8_t uart_num = 1;
const uint8_t led_pins[] = { 17 };
const uint8_t switch_pins[] = { 18 };
const uint8_t pwm_pins[] = { 2, 3 };
const uint8_t adc_pins[] = { 0, 1 };
