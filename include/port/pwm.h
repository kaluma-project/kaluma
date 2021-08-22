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

#ifndef __KM_PWM_H
#define __KM_PWM_H

#include <stdint.h>
#define KM_PWMPORT_ERROR -1
#define KM_PWM_DEFAULT_FREQUENCY 490
#define KM_PWM_DEFAULT_DUTY 1.0
#define KM_PWM_DUTY_MIN 0
#define KM_PWM_DUTY_MAX 1.0

/**
 * Initialize all PWM when system started
 */
void km_pwm_init();

/**
 * Cleanup all PWM when system cleanup
 */
void km_pwm_cleanup();

/**
 * Setup a PWM inversion pin
 *
 * @param {uint8_t} pin
 * @param {uint8_t} inv_pin
 * @return result status code
 */
int8_t km_pwm_set_inversion(uint8_t pin, uint8_t inv_pin);

/**
 * Setup a PWM channel
 *
 * @param {uint8_t} pin
 * @param {double} frequency
 * @param {double} duty
 * @return result status code
 */
int km_pwm_setup(uint8_t pin, double frequency, double duty);

/**
 * Check inv_pin can make inversion of the pin
 *
 * @param {uint8_t} pin
 * @param {int8_t} inv_pin, inversion pin, -1 when inv_pin is not used.
 * @return -1 if inv_pin can't generate inversion of the pin
 */
int km_check_pwm_inv_port(uint8_t pin, int8_t inv_pin);

/**
 * Start the PWM channel
 *
 * @param {uint8_t} pin
 */
int km_pwm_start(uint8_t pin);

/**
 * Stop the PWM channel
 *
 * @param {uint8_t} pin
 */
int km_pwm_stop(uint8_t pin);

/**
 * Return the current frequency
 *
 * @param {uint8_t} pin
 * @return {double}
 */
double km_pwm_get_frequency(uint8_t pin);

/**
 * Set the current frequency
 *
 * @param {uint8_t} pin
 * @param {double} frequency
 */
int km_pwm_set_frequency(uint8_t pin, double frequency);

/**
 * Return the current duty cycle
 *
 * @param {uint8_t} pin
 * @return {double}
 */
double km_pwm_get_duty(uint8_t pin);

/**
 * Set the current duty cycle
 *
 * @param {uint8_t} pin
 * @param {double} duty
 */
int km_pwm_set_duty(uint8_t pin, double duty);

/**
 * Close the PWM channel
 */
int km_pwm_close(uint8_t pin);

#endif /* __KM_PWM_H */
