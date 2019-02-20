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

#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>

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
void pwm_start(uint8_t pin);

/**
 * Stop the PWM channel
 *
 * @param {uint8_t} pin
 */
void pwm_stop(uint8_t pin);

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
void pwm_set_frequency(uint8_t pin, double frequency);

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
void pwm_set_duty(uint8_t pin, double duty);

/**
 * Close the PWM channel
 */
void pwm_close(uint8_t pin);

#endif /* __PWM_H */

