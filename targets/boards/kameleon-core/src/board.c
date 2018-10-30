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
#include "board.h"
#include "kameleon_core.h"

const char board_arch[] = "cortex-m4";
const char board_platform[] = "unknown";
const char board_name[] = "kameleon-core";

const uint8_t pin_num = GPIO_NUM;
const uint8_t led_num = LED_NUM;
const uint8_t button_num = BUTTON_NUM;
const uint8_t pwm_num = PWM_NUM;
const uint8_t adc_num = ADC_NUM;
const uint8_t i2c_num = I2C_NUM;
const uint8_t spi_num = SPI_NUM;
const uint8_t uart_num = UART_NUM;
const uint8_t led_pins[] = { 20 };
const uint8_t button_pins[] = { 21 };
const uint8_t pwm_pins[] = { 8, 9, 13, 14, 15, 16 };
const uint8_t adc_pins[] = { 2, 3, 4, 5, 10, 11, 12 };
