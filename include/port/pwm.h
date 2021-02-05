/* Copyright (c) 2017 Kalamu
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

#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>
#define PWMPORT_ERROR  -1
#define PWM_DEFAULT_FREQUENCY 490
#define PWM_DEFAULT_DUTY 1.0
#define PWM_DUTY_MIN  0
#define PWM_DUTY_MAX  1.0

/**
 * Initialize all PWM when system started
 */
void pwm_init();

/**
 * Cleanup all PWM when system cleanup
 */
void pwm_cleanup();

/**
 * Setup a PWM channel
 *
 * @param {uint8_t} pin
 * @param {double} frequency
 * @param {double} duty
 * @return result status code
 */
int pwm_setup(uint8_t pin, double frequency, double duty);

/**
 * Start the PWM channel
 *
 * @param {uint8_t} pin
 */
int pwm_start(uint8_t pin);

/**
 * Stop the PWM channel
 *
 * @param {uint8_t} pin
 */
int pwm_stop(uint8_t pin);

/**
 * Return the current frequency
 *
 * @param {uint8_t} pin
 * @return {double}
 */
double pwm_get_frequency(uint8_t pin);

/**
 * Set the current frequency
 *
 * @param {uint8_t} pin
 * @param {double} frequency
 */
int pwm_set_frequency(uint8_t pin, double frequency);

/**
 * Return the current duty cycle
 *
 * @param {uint8_t} pin
 * @return {double}
 */
double pwm_get_duty(uint8_t pin);

/**
 * Set the current duty cycle
 *
 * @param {uint8_t} pin
 * @param {double} duty
 */
int pwm_set_duty(uint8_t pin, double duty);

/**
 * Close the PWM channel
 */
int pwm_close(uint8_t pin);

#endif /* __PWM_H */

