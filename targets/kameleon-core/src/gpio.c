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
#include "stm32f4xx_hal.h"
#include "gpio.h"

/**
 * Initialze GPIO pins
 * ~ : means PWM (e.g. ~4, ~5, ~6...)
 * @ : means analog (e.g. @1, @2, @3, ...)
 */
uint16_t pins[] = {
  GPIO_PIN_0, // PIN 0
  GPIO_PIN_1, // PIN 1
  GPIO_PIN_2, // PIN 2
  GPIO_PIN_3, // PIN 3

  // TODO: Define more pins ...
 };


 struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOC, GPIO_PIN_0},
   {GPIOC, GPIO_PIN_1},
   {GPIOC, GPIO_PIN_2},
   {GPIOC, GPIO_PIN_3},
   {GPIOB, GPIO_PIN_13}   // Alive LED
 };

void gpio_pin_mode(uint8_t pin, gpio_mode_t mode) {
  // TODO:
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Pin = gpio_port_pin[pin].pin;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio_port_pin[pin].port, &GPIO_InitStruct);   
}

void gpio_write(uint8_t pin, uint8_t value) {
  // TODO:
  GPIO_PinState pin_state;

  if(value == GPIO_LOW){
    pin_state = GPIO_PIN_RESET;
  }
  else if(value == GPIO_HIGH){
    pin_state = GPIO_PIN_SET;
  }

  HAL_GPIO_WritePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin, pin_state);
}

void gpio_toggle(uint8_t pin) {
  // TODO:
  HAL_GPIO_TogglePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
}

uint8_t gpio_read(uint8_t pin) {
  // TODO:
  uint8_t val = 0;
  GPIO_PinState pin_state;
  pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);

  if(pin_state == GPIO_PIN_RESET){
    val = 0;
  }
  else if(pin_state == GPIO_PIN_SET){
    val = 1;
  }

  return val;
}
