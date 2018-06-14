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
#include "stm32f4discovery.h"

struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOC, GPIO_PIN_6},     // 0
   {GPIOC, GPIO_PIN_8},     // 1
   {GPIOC, GPIO_PIN_9},     // 2
   {GPIOA, GPIO_PIN_8},     // 3
   {GPIOA, GPIO_PIN_15},    // 4
   {GPIOC, GPIO_PIN_11},    // 5
   {GPIOD, GPIO_PIN_0},     // 6
   {GPIOD, GPIO_PIN_1},     // 7

   {GPIOD, GPIO_PIN_2},     // 8
   {GPIOD, GPIO_PIN_3},     // 9
   {GPIOD, GPIO_PIN_6},     // 10
   {GPIOD, GPIO_PIN_7},     // 11
   {GPIOB, GPIO_PIN_4},     // 12
   {GPIOB, GPIO_PIN_5},     // 13
   {GPIOB, GPIO_PIN_7},     // 14
   {GPIOB, GPIO_PIN_8},     // 15

   {GPIOE, GPIO_PIN_2},     // 16
   {GPIOE, GPIO_PIN_4},     // 17
   {GPIOE, GPIO_PIN_5},     // 18
   {GPIOE, GPIO_PIN_6},     // 19
   {GPIOC, GPIO_PIN_13},    // 20
   {GPIOC, GPIO_PIN_14},    // 21
   {GPIOC, GPIO_PIN_15},    // 22
   {GPIOC, GPIO_PIN_1},     // 23
   
   {GPIOC, GPIO_PIN_2},     // 24
   {GPIOA, GPIO_PIN_1},     // 25
   {GPIOA, GPIO_PIN_3},     // 26
   {GPIOA, GPIO_PIN_2},     // 27
   {GPIOC, GPIO_PIN_5},     // 28
   {GPIOC, GPIO_PIN_4},     // 29
   {GPIOB, GPIO_PIN_1},     // 30
   {GPIOB, GPIO_PIN_0},     // 31
   
   {GPIOB, GPIO_PIN_2},     // 32
   {GPIOE, GPIO_PIN_7},     // 33
   {GPIOE, GPIO_PIN_8},     // 34
   {GPIOE, GPIO_PIN_9},     // 35
   {GPIOE, GPIO_PIN_10},    // 36
   {GPIOE, GPIO_PIN_11},    // 37
   {GPIOE, GPIO_PIN_12},    // 38
   {GPIOE, GPIO_PIN_13},    // 39

   {GPIOE, GPIO_PIN_14},    // 40
   {GPIOE, GPIO_PIN_15},    // 41
   {GPIOB, GPIO_PIN_11},    // 42
   {GPIOB, GPIO_PIN_12},    // 43
   {GPIOB, GPIO_PIN_13},    // 44
   {GPIOB, GPIO_PIN_14},    // 45
   {GPIOB, GPIO_PIN_15},    // 46
   {GPIOD, GPIO_PIN_8},     // 47
   
   {GPIOD, GPIO_PIN_9},     // 48
   {GPIOD, GPIO_PIN_10},    // 49
   {GPIOD, GPIO_PIN_11},    // 50 
   {GPIOC, GPIO_PIN_7},     // 51 (START OF USED PINS)
   {GPIOA, GPIO_PIN_9},     // 52
   {GPIOA, GPIO_PIN_10},    // 53
   {GPIOA, GPIO_PIN_13},    // 54
   {GPIOA, GPIO_PIN_14},    // 55
   
   {GPIOC, GPIO_PIN_10},    // 56
   {GPIOC, GPIO_PIN_12},    // 57
   {GPIOD, GPIO_PIN_4},     // 58
   {GPIOD, GPIO_PIN_5},     // 59
   {GPIOB, GPIO_PIN_3},     // 60
   {GPIOB, GPIO_PIN_6},     // 61
   {GPIOB, GPIO_PIN_9},     // 62
   {GPIOE, GPIO_PIN_0},     // 63
   
   {GPIOE, GPIO_PIN_1},     // 64
   {GPIOE, GPIO_PIN_3},     // 65
   {GPIOH, GPIO_PIN_0},     // 66
   {GPIOH, GPIO_PIN_1},     // 67
   {GPIOC, GPIO_PIN_0},     // 68
   {GPIOC, GPIO_PIN_3},     // 69
   {GPIOA, GPIO_PIN_0},     // 70 (BUTTON)
   {GPIOA, GPIO_PIN_5},     // 71
   
   {GPIOA, GPIO_PIN_4},     // 72
   {GPIOA, GPIO_PIN_7},     // 73
   {GPIOA, GPIO_PIN_6},     // 74
   {GPIOB, GPIO_PIN_10},    // 75
   {GPIOD, GPIO_PIN_12},    // 76 (LED4)
   {GPIOD, GPIO_PIN_13},    // 77 (LED3)
   {GPIOD, GPIO_PIN_14},    // 78 (LED5)
   {GPIOD, GPIO_PIN_15},    // 79 (LED6)
};

GPIO_TypeDef * get_gpio_port(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  return gpio_port_pin[pin].port;  
}

uint32_t get_gpio_pin(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  return gpio_port_pin[pin].pin;  
}
