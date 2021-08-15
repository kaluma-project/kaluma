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

#include "pwm.h"

#include <stdint.h>

/**
 * Initialize all PWM when system started
 */
void km_pwm_init() {}

/**
 * Cleanup all PWM when system cleanup
 */
void km_pwm_cleanup() {}

/**
 * return Returns 0 on success or -1 on failure.
 */
int km_pwm_setup(uint8_t pin, int8_t inv_pin, double frequency, double duty) {
  return 0;
}

int km_check_pwm_inv_port(uint8_t pin, int8_t inv_pin) {
  (void)pin;
  (void)inv_pin;
  return KM_PWMPORT_ERROR;
}

/**
 */
int km_pwm_start(uint8_t pin) { return 0; }

/**
 */
int km_pwm_stop(uint8_t pin) { return 0; }

/**
 */
double km_pwm_get_frequency(uint8_t pin) { return 0; }

/**
 */
double km_pwm_get_duty(uint8_t pin) { return 0; }

/**
 */
int km_pwm_set_duty(uint8_t pin, double duty) { return 0; }

/**
 */
int km_pwm_set_frequency(uint8_t pin, double frequency) { return 0; }

/**
 */
int km_pwm_close(uint8_t pin) { return 0; }
