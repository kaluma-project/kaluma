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

#include <stdint.h>
#include "adc.h"
#include "rpi_pico.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

/**
 * Get ADC index
 *
 * @param pin Pin number.
 * @return Returns index on success or -1 on failure.
 */

/**
 * input : pinNumber
 * output : adc channel or -1 if the pin does not support ADC
*/
static int get_adc_index(uint8_t pin) {
  if ((pin >= 26) && (pin <= 28)) {
    return pin - 26; //GPIO 26 is channel 0
  }
  return -1; // Error
}

/**
 * Initialize all ADC channels when system started
 */
void km_adc_init() {
  adc_init();
}

/**
 * Cleanup all ADC channels when system cleanup
 */
void km_adc_cleanup() {
  // adc pins will be reset at the GPIO cleanup function.
}

/**
 * Read value from the ADC channel
 *
 * @param {uint8_t} adcIndex
 * @return {double}
 */
double km_adc_read(uint8_t adcIndex) {
  return (double)adc_read() / (1 << ADC_RESOLUTION_BIT);
}

int km_adc_setup(uint8_t pin) {
  int ch = get_adc_index(pin);
  if (ch < 0) {
    return -1; // Error
  }
  adc_gpio_init(pin);
  adc_select_input(ch);
  return 0;
}

int km_adc_close(uint8_t pin) {
  if (get_adc_index(pin) < 0) {
    return -1; // Error
  }
  return 0;
}

