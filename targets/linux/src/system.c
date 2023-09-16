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

#include "system.h"

#include <time.h>

#include "adc.h"
#include "flash.h"
#include "gpio.h"
#include "i2c.h"
#include "pwm.h"
#include "rtc.h"
#include "spi.h"
#include "tty.h"
#include "uart.h"

const char km_system_arch[] = "i686";
const char km_system_platform[] = "linux";

/**
 */
void km_delay(uint32_t msec) {}

/**
 */
uint64_t km_gettime() {
  // struct timeval tval;
  // gettimeofday(&tval, NULL);
  struct timespec tval;
  clock_gettime(CLOCK_MONOTONIC, &tval);
  long t = (tval.tv_sec * 1000) + (tval.tv_nsec / 1000000);
  return (uint64_t)t;
}

/**
 */
char *km_getuid() {
  return "";
}


/**
 * Return MAX of the micro seconde counter 44739242
 */
uint64_t km_micro_maxtime() { return UINT32_MAX; }

/**
 * Return micro seconde counter
 */
uint64_t km_micro_gettime() {
  struct timespec tval;
  clock_gettime(CLOCK_MONOTONIC, &tval);
  long t = (tval.tv_sec * 1000000) + (tval.tv_nsec / 1000);
  return (uint64_t)t;
}

/**
 * micro secoded delay
 */
void km_micro_delay(uint32_t usec) {}

/**
 * Kaluma Hardware System Initializations
 */
void km_system_init() {
  km_gpio_init();
  km_adc_init();
  km_pwm_init();
  km_i2c_init();
  km_spi_init();
  km_uart_init();
  km_rtc_init();
  km_flash_init();
}

void km_system_cleanup() {
  km_adc_cleanup();
  km_pwm_cleanup();
  km_i2c_cleanup();
  km_spi_cleanup();
  km_uart_cleanup();
  km_gpio_cleanup();
  km_rtc_cleanup();
  km_flash_cleanup();
}

uint8_t km_running_script_check() { return false; }

void km_custom_infinite_loop() {}
