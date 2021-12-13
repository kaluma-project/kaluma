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

#include "adc.h"

#include <stdint.h>

/**
 * Get ADC index
 *
 * @param pin Pin number.
 * @return Returns index on success or -1 on failure.
 */

// /**
//  * input : pinNumber
//  * output : pinIndex or ADC_PORTERRROR (-1)
//  *          0xFF means the pin is not assigned for ADC
// */
// static int get_adc_index(uint8_t pin) {
//   return 0;
// }

/**
 * Initialize all ADC channels when system started
 */
void km_adc_init() {}

/**
 * Cleanup all ADC channels when system cleanup
 */
void km_adc_cleanup() {}

/**
 * Read value from the ADC channel
 *
 * @param {uint8_t} adcIndex
 * @return {double}
 */
double km_adc_read(uint8_t adcIndex) { return 0.0; }

int km_adc_setup(uint8_t pin) { return 0; }

int km_adc_close(uint8_t pin) { return 0; }
