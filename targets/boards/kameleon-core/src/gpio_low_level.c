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
/**
 * Initialze GPIO pins
 * ~ : means PWM (e.g. ~4, ~5, ~6...)
 * @ : means analog (e.g. @1, @2, @3, ...)
 */
#include "stm32f4xx.h"
#include "kameleon_core.h"

struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOB, GPIO_PIN_0},     // 0
   {GPIOB, GPIO_PIN_1},     // 1
   {GPIOA, GPIO_PIN_0},     // 2
   {GPIOA, GPIO_PIN_1},     // 3
   {GPIOA, GPIO_PIN_2},     // 4
   {GPIOA, GPIO_PIN_3},     // 5
   {GPIOA, GPIO_PIN_9},     // 6
   {GPIOA, GPIO_PIN_10},    // 7

   {GPIOB, GPIO_PIN_6},     // 8
   {GPIOB, GPIO_PIN_7},     // 9
   {GPIOA, GPIO_PIN_5},     // 10
   {GPIOA, GPIO_PIN_6},     // 11
   {GPIOA, GPIO_PIN_7},     // 12
   {GPIOB, GPIO_PIN_4},     // 13
   {GPIOB, GPIO_PIN_5},     // 14
   {GPIOB, GPIO_PIN_10},    // 15

   {GPIOB, GPIO_PIN_9},     // 16
   {GPIOB, GPIO_PIN_3},     // 17
   {GPIOB, GPIO_PIN_12},    // 18
   {GPIOC, GPIO_PIN_15},    // 19
   {GPIOB, GPIO_PIN_14},    // 20 (LED)
   {GPIOA, GPIO_PIN_4},     // 21 (BUTTON)
};

GPIO_TypeDef * get_gpio_port(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  return gpio_port_pin[pin].port;  
}

uint32_t get_gpio_pin(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  return gpio_port_pin[pin].pin;  
}
