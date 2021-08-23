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
#include "gpio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pll.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/xosc.h"
#include "i2c.h"
#include "io.h"
#include "pico/stdlib.h"
#include "pwm.h"
#include "rpi_pico.h"
#include "spi.h"
#include "tty.h"
#include "tusb.h"
#include "uart.h"

const char km_system_arch[] = "cortex-m0-plus";
const char km_system_platform[] = "unknown";

/**
 */
void km_delay(uint32_t msec) { sleep_ms(msec); }

/**
 */
uint64_t km_gettime() { return to_ms_since_boot(get_absolute_time()); }

/**
 * Return MAX of the micro seconde counter 44739242
 */
uint64_t km_micro_maxtime() {
  return 0xFFFFFFFFFFFFFFFF;  // Max of the uint64()
}
/**
 * Return micro seconde counter
 */
uint64_t km_micro_gettime() { return get_absolute_time(); }

/**
 * micro secoded delay
 */
void km_micro_delay(uint32_t usec) { sleep_us(usec); }

int km_enter_dormant(km_dormant_event_t *param_arr, uint8_t length) {
  if (length >= NUM_BANK0_GPIOS) {
    return KM_DORMANT_LENGTH_ERROR;  // Error
  }
  for (int i = 0; i < length; i++) {
    if (param_arr[i].pin >= NUM_BANK0_GPIOS) {
      return KM_DORMANT_PARAM_ERROR;  // Error
    }
  }

  uint32_t _event[length];
  if (tud_ready()) {
    /* Just return if USB is connected */
    return 0;
  }

  uint src_hz = XOSC_MHZ * MHZ;
  uint clk_ref_src = CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC;

  // CLK_REF = XOSC
  clock_configure(clk_ref, clk_ref_src,
                  0,  // No aux mux
                  src_hz, src_hz);

  // CLK SYS = CLK_REF
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                  0,  // Using glitchless mux
                  src_hz, src_hz);

  // CLK USB = 0MHz
  clock_stop(clk_usb);

  // CLK ADC = 0MHz
  clock_stop(clk_adc);

  // CLK RTC = ideally XOSC (12MHz) / 256 = 46875Hz but could be rosc
  uint clk_rtc_src = CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC;

  clock_configure(clk_rtc, 0,  // No GLMUX
                  clk_rtc_src, src_hz, 46875);

  // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so
  // just select and enable
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  src_hz, src_hz);

  pll_deinit(pll_sys);
  pll_deinit(pll_usb);

  for (int i = 0; i < length; i++) {
    switch (param_arr[i].event) {
      case KM_IO_WATCH_MODE_LOW_LEVEL:
        _event[i] = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS;
        break;
      case KM_IO_WATCH_MODE_HIGH_LEVEL:
        _event[i] = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_HIGH_BITS;
        break;
      case KM_IO_WATCH_MODE_RISING:
        _event[i] = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS;
        break;
      case KM_IO_WATCH_MODE_FALLING:
        _event[i] = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS;
        break;
      case KM_IO_WATCH_MODE_CHANGE:
        _event[i] = (IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS |
                     IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS);
        break;
      default:
        return KM_GPIOPORT_ERROR;  // Event error
    }
    gpio_set_dormant_irq_enabled(param_arr[i].pin, _event[i], true);
  }
  xosc_dormant();
  // Execution stops here until woken up
  for (int i = 0; i < length; i++) {
    // Clear the irq so we can go back to dormant mode again if we want
    gpio_acknowledge_irq(param_arr[i].pin, _event[i]);
  }
  /* Resume the system */
  pll_init(pll_sys, 1, 1500 * MHZ, 6, 2);
  pll_init(pll_usb, 1, 480 * MHZ, 5, 2);

  // Configure clocks
  // CLK_REF = XOSC (12MHz) / 1 = 12MHz
  clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                  0,  // No aux mux
                  12 * MHZ, 12 * MHZ);

  /// \tag::configure_clk_sys[]
  // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                  CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 125 * MHZ,
                  125 * MHZ);
  /// \end::configure_clk_sys[]

  // CLK USB = PLL USB (48MHz) / 1 = 48MHz
  clock_configure(clk_usb,
                  0,  // No GLMUX
                  CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ,
                  48 * MHZ);

  // CLK ADC = PLL USB (48MHZ) / 1 = 48MHz
  clock_configure(clk_adc,
                  0,  // No GLMUX
                  CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ,
                  48 * MHZ);

  // CLK RTC = PLL USB (48MHz) / 1024 = 46875Hz
  clock_configure(clk_rtc,
                  0,  // No GLMUX
                  CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ,
                  46875);

  // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so
  // just select and enable Normally choose clk_sys or clk_usb
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  125 * MHZ, 125 * MHZ);
  return 0;
}

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
}

void km_system_cleanup() {
  km_adc_cleanup();
  km_pwm_cleanup();
  km_i2c_cleanup();
  km_spi_cleanup();
  km_uart_cleanup();
  km_gpio_cleanup();
}

uint8_t km_running_script_check() {
  gpio_set_pulls(SCR_LOAD_GPIO, true, false);
  sleep_us(100);
  bool load_state = gpio_get(SCR_LOAD_GPIO);
  gpio_set_pulls(SCR_LOAD_GPIO, false, false);
  return load_state;
}
