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

#include "adc.h"
#include "board.h"
#include "flash.h"
#include "gpio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pll.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/xosc.h"
#include "hardware/pio.h"
#include "i2c.h"
#include "io.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pwm.h"
#include "rtc.h"
#include "spi.h"
#include "tty.h"
#include "tusb.h"
#include "uart.h"
#ifdef PICO_CYW43
#include "module_pico_cyw43.h"
#include <pico/cyw43_arch.h>
#endif /* PICO_CYW43 */

static char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

/**
 * Delay in milliseconds
 */
void km_delay(uint32_t msec) { sleep_ms(msec); }

/**
 * Return current time (UNIX timestamp in milliseconds)
 */
uint64_t km_gettime() { return to_ms_since_boot(get_absolute_time()); }

/**
 * Return uid of device
 */
char *km_getuid() {
  return serial;
}

/**
 * Return MAX of the microsecond counter 44739242
 */
uint64_t km_micro_maxtime() {
  return 0xFFFFFFFFFFFFFFFF;  // Max of the uint64()
}

/**
 * Return microsecond counter
 */
#ifdef NDEBUG
uint64_t km_micro_gettime() { return get_absolute_time(); }
#else
uint64_t km_micro_gettime() { return get_absolute_time()._private_us_since_boot; }
#endif

/**
 * microsecond delay
 */
void km_micro_delay(uint32_t usec) { sleep_us(usec); }

static void km_uid_init() {
  pico_get_unique_board_id_string(serial, sizeof(serial));
}

static void km_pio_init() {
  for (int i = 0; i < KALUMA_PIO_SM_NUM; i++) {
    if (pio_sm_is_claimed(pio0, i))
      pio_sm_unclaim(pio0, i);
    if (pio_sm_is_claimed(pio1, i))
      pio_sm_unclaim(pio1, i);
  }
  pio_clear_instruction_memory(pio0);
  pio_clear_instruction_memory(pio1);
}

/**
 * Kaluma Hardware System Initializations
 */
void km_system_init() {
  stdio_init_all();
  km_uid_init();
  km_pio_init();
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
#ifdef PICO_CYW43
  km_cyw43_deinit();
#endif
  km_adc_cleanup();
  km_pwm_cleanup();
  km_i2c_cleanup();
  km_spi_cleanup();
  km_uart_cleanup();
  km_gpio_cleanup();
  km_rtc_cleanup();
  km_flash_cleanup();
}

uint8_t km_running_script_check() {
  gpio_set_pulls(SCR_LOAD_GPIO, true, false);
  sleep_us(100);
  bool load_state = gpio_get(SCR_LOAD_GPIO);
  gpio_set_pulls(SCR_LOAD_GPIO, false, false);
  return load_state;
}

void km_custom_infinite_loop() {
#ifdef PICO_CYW43
  km_cyw43_infinite_loop();
#endif
}
