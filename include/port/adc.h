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

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>
#define ADCPORT_ERRROR  -1

/**
 * Initialize all ADC channels when system started
 */
void adc_init();

/**
 * Cleanup all ADC channels when system cleanup
 */
void adc_cleanup();

/**
 * Setup a ADC channel
 *
 * @param pin Pin number.
 * @return Returns channel number on success or -1 on failure.
 */
int adc_setup(uint8_t pin);

/**
 * Read value from a ADC channel
 *
 * @param ADC index (output of adc_setup).
 * @return Return a value read between 0 and 1.
 */
double adc_read(uint8_t adcIndex);

/**
 * Close the ADC channel
 *
 * @param pin Pin number.
 * @return Returns 0 on success or -1 on failure.
 */
int adc_close(uint8_t pin);

#endif /* __ADC_H */
