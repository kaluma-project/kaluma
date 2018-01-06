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

#include <stdbool.h>
#include "target.h"
#include "board.h"

bool is_led_pin(uint8_t pin) {
  for (int i = 0; i < led_num; i++) {
    if (pin == led_pins[i]) {
      return true;
    }
  }
  return false;
}

bool is_switch_pin(uint8_t pin) {
  for (int i = 0; i < switch_num; i++) {
    if (pin == switch_pins[i]) {
      return true;
    }
  }
  return false;
}

bool is_pwm_pin(uint8_t pin) {
  for (int i = 0; i < pwm_num; i++) {
    if (pin == pwm_pins[i]) {
      return true;
    }
  }
  return false;
}

bool is_adc_pin(uint8_t pin) {
  for (int i = 0; i < adc_num; i++) {
    if (pin == adc_pins[i]) {
      return true;
    }
  }
  return false;
}
