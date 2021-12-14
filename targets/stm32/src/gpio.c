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

#include "board.h"
#include "stm32f4xx.h"

const struct {
  GPIO_TypeDef* port;
  uint32_t pin;
} gpio_port_pin[] = {
    {GPIOB, GPIO_PIN_0},   // 0
    {GPIOB, GPIO_PIN_1},   // 1
    {GPIOA, GPIO_PIN_0},   // 2
    {GPIOA, GPIO_PIN_1},   // 3
    {GPIOA, GPIO_PIN_2},   // 4
    {GPIOA, GPIO_PIN_3},   // 5
    {GPIOA, GPIO_PIN_9},   // 6
    {GPIOA, GPIO_PIN_10},  // 7

    {GPIOB, GPIO_PIN_6},   // 8
    {GPIOB, GPIO_PIN_7},   // 9
    {GPIOA, GPIO_PIN_5},   // 10
    {GPIOA, GPIO_PIN_6},   // 11
    {GPIOA, GPIO_PIN_7},   // 12
    {GPIOB, GPIO_PIN_4},   // 13
    {GPIOB, GPIO_PIN_5},   // 14
    {GPIOB, GPIO_PIN_10},  // 15

    {GPIOB, GPIO_PIN_9},   // 16
    {GPIOB, GPIO_PIN_3},   // 17
    {GPIOB, GPIO_PIN_12},  // 18
    {GPIOC, GPIO_PIN_15},  // 19
    {GPIOB, GPIO_PIN_14},  // 20 (LED)
    {GPIOA, GPIO_PIN_4},   // 21 (BUTTON)
};

/**
 * Initialize all GPIO when system started
 */
void km_gpio_init() {
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 |
                             GPIO_PIN_10);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 |
                             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 |
                             GPIO_PIN_10 | GPIO_PIN_12);
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_15);
}

/**
 * Cleanup all GPIO when system cleanup
 */
void km_gpio_cleanup() { km_gpio_init(); }

/**
 */
int km_gpio_set_io_mode(uint8_t pin, km_gpio_io_mode_t mode) {
  if (pin >= KALUMA_GPIO_COUNT) return KM_GPIOPORT_ERROR;
  if ((mode != KM_GPIO_IO_MODE_INPUT) && (mode != KM_GPIO_IO_MODE_INPUT_PULLUP))
    mode = KM_GPIO_IO_MODE_OUTPUT;
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = gpio_port_pin[pin].pin;
  GPIO_InitStruct.Mode =
      (mode == KM_GPIO_IO_MODE_OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull =
      (mode == KM_GPIO_IO_MODE_INPUT_PULLUP) ? GPIO_PULLUP : GPIO_NOPULL;
  GPIO_InitStruct.Pull = (mode == KM_GPIO_IO_MODE_INPUT_PULLDOWN)
                             ? GPIO_PULLDOWN
                             : GPIO_InitStruct.Pull;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio_port_pin[pin].port, &GPIO_InitStruct);
  return 0;
}

/**
 */
int km_gpio_write(uint8_t pin, uint8_t value) {
  if (pin >= KALUMA_GPIO_COUNT) return KM_GPIOPORT_ERROR;
  if (value != KM_GPIO_LOW) value = KM_GPIO_HIGH;
  GPIO_PinState pin_state =
      (value == KM_GPIO_LOW) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin, pin_state);
  return 0;
}

/**
 */
int km_gpio_read(uint8_t pin) {
  if (pin >= KALUMA_GPIO_COUNT) return KM_GPIOPORT_ERROR;
  GPIO_PinState pin_state =
      HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  return (pin_state == GPIO_PIN_RESET) ? KM_GPIO_LOW : KM_GPIO_HIGH;
}

/**
 */
int km_gpio_toggle(uint8_t pin) {
  if (pin >= KALUMA_GPIO_COUNT) return KM_GPIOPORT_ERROR;
  HAL_GPIO_TogglePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  return 0;
}

void km_gpio_intr_en(bool en, km_gpio_callback_t call_back) {}

int km_gpio_set_interrupt(bool en, uint8_t pin, uint8_t events) {
  return KM_GPIOPORT_ERROR;
}
