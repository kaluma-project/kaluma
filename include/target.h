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

#ifndef __TARGET_H
#define __TARGET_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  BOARD_PIN_FUNCTION_GPIO_INPUT,
  BOARD_PIN_FUNCTION_GPIO_OUTPUT,
  BOARD_PIN_FUNCTION_ADC,
  BOARD_PIN_FUNCTION_PWM,
  BOARD_PIN_FUNCTION_I2C,
  BOARD_PIN_FUNCTION_SPI,
  BOARD_PIN_FUNCTION_UART
} board_pin_function_t;

typedef struct {
  board_pin_function_t function;
  uint8_t bus;  // Used to denote bus number for I2C, SPI, UART
} board_pin_state_t;

extern board_pin_state_t board_pin_states[];

void board_init();
void board_deinit();

bool is_led_pin(uint8_t pin);
bool is_switch_pin(uint8_t pin);
bool is_pwm_pin(uint8_t pin);
bool is_adc_pin(uint8_t pin);
bool is_uart_bus(uint8_t bus);
bool is_i2c_bus(uint8_t bus);
bool is_spi_bus(uint8_t bus);

#endif /* __TARGET_H */