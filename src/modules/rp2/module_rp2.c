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

#include <stdlib.h>

#include "board.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pll.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/xosc.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "rp2_magic_strings.h"

#define __PIO_INT_EN_PIO0_0 1
#define __PIO_INT_EN_PIO0_1 2
#define __PIO_INT_EN_PIO1_0 4
#define __PIO_INT_EN_PIO1_1 8

#define __PIO_INT_SM0 0x100
#define __PIO_INT_SM1 0x200
#define __PIO_INT_SM2 0x400
#define __PIO_INT_SM3 0x800
#define __PIO_INT_FULL 0xF00

#define __RP2_TEMP_ADC_PORT 30

static jerry_value_t __pio_call_back[KALUMA_PIO_NUM];

static PIO __pio(uint8_t pio) {
  if (pio == 0) {
    return pio0;
  } else if (pio == 1) {
    return pio1;
  }
  return NULL;
}

void __pio_handler(uint8_t pio, uint8_t interrupt) {
  if (jerry_value_is_function(__pio_call_back[pio])) {
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t args[1];
    args[0] = jerry_number(interrupt);
    jerry_value_t ret_val =
        jerry_call(__pio_call_back[pio], this_val, args, 1);
    if (jerry_value_is_error(ret_val)) {
      // print error
      jerryxx_print_error(ret_val, true);
    }
    jerry_value_free(ret_val);
    jerry_value_free(this_val);
  }
}

void __pio0_irq_0_handler(void) {
  PIO _pio = __pio(0);
  uint32_t ints = _pio->ints0;
  ints >>= 8;
  _pio->irq = ints;
  __pio_handler(0, ints);
}

void __pio1_irq_0_handler(void) {
  PIO _pio = __pio(1);
  uint32_t ints = _pio->ints0;
  ints >>= 8;
  _pio->irq = ints;
  __pio_handler(1, ints);
}

JERRYXX_FUN(pio_add_program_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG(1, "prog");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t prog = JERRYXX_GET_ARG(1);
  pio_program_t pio_prog;
  if (jerry_value_is_typedarray(prog) &&
      jerry_typedarray_type(prog) ==
          JERRY_TYPEDARRAY_UINT16) { /* Uint16Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(prog, &byteOffset, &byteLength);
    pio_prog.origin = -1;
    pio_prog.pio_version = PICO_PIO_VERSION;
    pio_prog.instructions =
        (uint16_t *)jerry_arraybuffer_data(array_buffer),
    pio_prog.length = jerry_arraybuffer_size(array_buffer) / 2,
    jerry_value_free(array_buffer);
    PIO _pio = __pio(pio);
    int offset = pio_add_program(_pio, &pio_prog);
    return jerry_number(offset);
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The prog argument must be Uint16Array");
  }
}

JERRYXX_FUN(pio_sm_init_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_OBJECT(2, "options");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  jerry_value_t options = JERRYXX_GET_ARG(2);
  pio_sm_config sm_config = pio_get_default_sm_config();
  PIO _pio = __pio(pio);
  uint32_t freq = (uint32_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_FREQ, 125000000);
  float div = clock_get_hz(clk_sys) / freq;
  sm_config_set_clkdiv(&sm_config, div);
  // setup in pins
  int8_t in_base =
      (int8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_IN_BASE, -1);
  if (in_base >= 0) {
    uint8_t in_count = (uint8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_IN_COUNT, 1);
    sm_config_set_in_pins(&sm_config, in_base);
    pio_sm_set_consecutive_pindirs(_pio, sm, in_base, in_count, false);
    for (int i = 0; i < in_count; i++) {
      pio_gpio_init(_pio, in_base + i);
    }
  }
  // setup out pins
  int8_t out_base = (int8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUT_BASE, -1);
  if (out_base >= 0) {
    uint8_t out_count = (uint8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_OUT_COUNT, 1);
    sm_config_set_out_pins(&sm_config, out_base, out_count);
    pio_sm_set_consecutive_pindirs(_pio, sm, out_base, out_count, true);
    for (int i = 0; i < out_count; i++) {
      pio_gpio_init(_pio, out_base + i);
    }
  }
  // setup set pins
  int8_t set_base = (int8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_SET_BASE, -1);
  if (set_base >= 0) {
    uint8_t set_count = (uint8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_SET_COUNT, 1);
    sm_config_set_set_pins(&sm_config, set_base, set_count);
    pio_sm_set_consecutive_pindirs(_pio, sm, set_base, set_count, true);
    for (int i = 0; i < set_count; i++) {
      pio_gpio_init(_pio, set_base + i);
    }
  }
  // setup sideset pins
  bool sideset = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_SIDESET, false);
  if (sideset) {
    int8_t sideset_base = (int8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_SIDESET_BASE, -1);
    if (sideset_base >= 0) {
      uint8_t sideset_bits = (uint8_t)jerryxx_get_property_number(
          options, MSTR_RP2_PIO_SM_SIDESET_BITS, 1);
      bool sideset_opt = (uint8_t)jerryxx_get_property_boolean(
          options, MSTR_RP2_PIO_SM_SIDESET_OPT, false);
      bool sideset_pindirs = (uint8_t)jerryxx_get_property_boolean(
          options, MSTR_RP2_PIO_SM_SIDESET_PINDIRS, false);
      pio_sm_set_consecutive_pindirs(_pio, sm, sideset_base, sideset_bits,
                                     true);
      for (int i = 0; i < sideset_bits; i++) {
        pio_gpio_init(_pio, sideset_base + i);
      }
      sm_config_set_sideset_pins(&sm_config, sideset_base);
      if (sideset_opt) {
        sideset_bits++;  // Add 1 bit for option.
      }
      sm_config_set_sideset(&sm_config, sideset_bits, sideset_opt,
                            sideset_pindirs);
    }
  }
  // setup jmp pin
  int8_t jmp_pin =
      (int8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_JMP_PIN, -1);
  if (jmp_pin >= 0) {
    sm_config_set_jmp_pin(&sm_config, jmp_pin);
  }
  // setup wrap
  uint8_t wrap_target = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_WRAP_TARGET, 0);
  uint8_t wrap =
      (uint8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_WRAP, 31);
  int offset =
      (int)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_OFFSET, 0);
  sm_config_set_wrap(&sm_config, offset + wrap_target, offset + wrap);
  // setup in-shift
  uint8_t inshift_dir = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_INSHIFT_DIR, 1);
  bool autopush = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_AUTOPUSH, 0);
  uint8_t push_threshold = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_PUSH_THRESHOLD, 32);
  sm_config_set_in_shift(&sm_config, inshift_dir, autopush, push_threshold);
  // setup out-shift
  uint8_t outshift_dir = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUTSHIFT_DIR, 1);
  bool autopull = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_AUTOPULL, 0);
  uint8_t pull_threshold = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_PULL_THRESHOLD, 32);
  sm_config_set_out_shift(&sm_config, outshift_dir, autopull, pull_threshold);
  // setup fifoJoin
  uint8_t fifo_join = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_FIFO_JOIN, 0);
  sm_config_set_fifo_join(&sm_config, fifo_join);
  // setup out special
  bool out_sticky = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_OUT_STICKY, false);
  int8_t out_enable_pin = (int8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUT_ENABLE_PIN, -1);
  sm_config_set_out_special(&sm_config, out_sticky, (out_enable_pin > -1),
                            (out_enable_pin > -1) ? out_enable_pin : 0);
  // setup mov status
  uint8_t move_status_sel = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_MOV_STATUS_SEL, 0);
  uint8_t move_status_n = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_MOV_STATUS_N, 0);
  sm_config_set_mov_status(&sm_config, move_status_sel, move_status_n);

  pio_sm_init(_pio, sm, offset, &sm_config);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_set_enabled_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_BOOLEAN(2, "enabled");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  bool enabled = JERRYXX_GET_ARG_BOOLEAN(2);
  PIO _pio = __pio(pio);
  pio_sm_set_enabled(_pio, sm, enabled);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_restart_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  pio_sm_restart(_pio, sm);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_exec_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_NUMBER(2, "inst");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t inst = (uint16_t)JERRYXX_GET_ARG_NUMBER(2);
  PIO _pio = __pio(pio);
  pio_sm_exec(_pio, sm, inst);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_put_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG(2, "data");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  jerry_value_t data = JERRYXX_GET_ARG(2);
  PIO _pio = __pio(pio);
  if (jerry_value_is_typedarray(data) &&
      jerry_typedarray_type(data) ==
          JERRY_TYPEDARRAY_UINT32) { /* Uint32Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_typedarray_buffer(data, &byteOffset, &byteLength);
    size_t len = jerry_arraybuffer_size(array_buffer) / 4;
    uint8_t *data_buf = jerry_arraybuffer_data(array_buffer);
    jerry_value_free(array_buffer);
    for (int i = 0; i < len; i++) {
      pio_sm_put_blocking(_pio, sm, *((uint32_t *)data_buf + i));
    }
  } else if (jerry_value_is_number(data)) {
    uint32_t data_value = jerry_value_as_number(data);
    pio_sm_put_blocking(_pio, sm, data_value);
  } else {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The data argument must be number of Uint32Array");
  }
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_get_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  uint32_t data = pio_sm_get_blocking(_pio, sm);
  return jerry_number(data);
}

JERRYXX_FUN(pio_sm_set_pins_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_NUMBER(2, "value");
  JERRYXX_CHECK_ARG_NUMBER(3, "mask");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint32_t value = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  uint32_t mask = (uint32_t)JERRYXX_GET_ARG_NUMBER(3);
  PIO _pio = __pio(pio);
  pio_sm_set_pins_with_mask(_pio, sm, value, mask);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_rxfifo_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  uint length = pio_sm_get_rx_fifo_level(_pio, sm);
  return jerry_number(length);
}

JERRYXX_FUN(pio_sm_txfifo_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  uint length = pio_sm_get_tx_fifo_level(_pio, sm);
  return jerry_number(length);
}

JERRYXX_FUN(pio_sm_clear_fifos_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  pio_sm_clear_fifos(_pio, sm);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_drain_txfifo_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  pio_sm_drain_tx_fifo(_pio, sm);
  return jerry_undefined();
}

JERRYXX_FUN(pio_sm_irq_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_FUNCTION(1, "callback");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t callback = JERRYXX_GET_ARG(1);
  PIO _pio = __pio(pio);
  if (pio == 0) {
    _pio->inte0 = __PIO_INT_FULL;
    irq_set_mask_enabled((1u << PIO0_IRQ_0), true);
    irq_set_enabled(PIO0_IRQ_0, true);
  } else {
    _pio->inte0 = __PIO_INT_FULL;
    irq_set_mask_enabled((1u << PIO1_IRQ_0), true);
    irq_set_enabled(PIO1_IRQ_0, true);
  }
  __pio_call_back[pio] = jerry_value_copy(callback);

  return jerry_undefined();
}

JERRYXX_FUN(dormant_fn) {
  JERRYXX_CHECK_ARG_ARRAY(0, "pins");
  JERRYXX_CHECK_ARG_ARRAY(1, "events");
  jerry_value_t pins = JERRYXX_GET_ARG(0);
  jerry_value_t events = JERRYXX_GET_ARG(1);
  int len = jerry_array_length(pins);
  int elen = jerry_array_length(events);
  if (len != elen) {
    return jerry_error_sz(
        JERRY_ERROR_TYPE,
        "The length of pins and events should be the same.");
  }

  uint8_t _pins[len];
  uint8_t _events[elen];
  for (int i = 0; i < len; i++) {
    jerry_value_t pin = jerry_object_get_index(pins, i);
    jerry_value_t event = jerry_object_get_index(events, i);
    if (!jerry_value_is_number(pin)) {
      return jerry_error_sz(
          JERRY_ERROR_TYPE,
          "The pin should be a number.");
    }
    if (!jerry_value_is_number(event)) {
      return jerry_error_sz(
          JERRY_ERROR_TYPE,
          "The event should be a number.");
    }
    _pins[i] = (uint8_t)jerry_value_as_number(pin);
    _events[i] = (uint8_t)jerry_value_as_number(event);
    jerry_value_free(pin);
    jerry_value_free(event);
  }

  // Hibernate system for dormant
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
#ifdef PICO_RP2040
  // CLK RTC = ideally XOSC (12MHz) / 256 = 46875Hz but could be rosc
  uint clk_rtc_src = CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC;
  clock_configure(clk_rtc, 0,  // No GLMUX
                  clk_rtc_src, src_hz, 46875);
#endif
  // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so
  // just select and enable
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  src_hz, src_hz);
  pll_deinit(pll_sys);
  pll_deinit(pll_usb);

  // Setup GPIOs for wakeup
  for (int i = 0; i < len; i++) {
    gpio_set_dormant_irq_enabled(_pins[i], _events[i], true);
  }

  // Enter dormant state
  xosc_dormant();

  // Execution stops here until woken up
  for (int i = 0; i < len; i++) {
    // Clear the irq so we can go back to dormant mode again if we want
    gpio_acknowledge_irq(_pins[i], _events[i]);
  }

  // Resume the system
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
#ifdef PICO_RP2040
  clock_configure(clk_rtc,
                  0,  // No GLMUX
                  CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ,
                  46875);
#endif
  // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so
  // just select and enable Normally choose clk_sys or clk_usb
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  125 * MHZ, 125 * MHZ);
  return jerry_undefined();
}

/**
 * random_big_int_fn()
 * args:
 *   min {number}, optional
 *   max {number}
 *   callback {function}
 */
JERRYXX_FUN(random_big_int_fn) {
  rng_128_t random;
  get_rand_128 (&random);
  return jerry_bigint(random.r, 2, false);
}

/**
 * Initialize 'rp2' module and return exports
 */
jerry_value_t module_rp2_init() {
  irq_set_exclusive_handler(PIO0_IRQ_0, __pio0_irq_0_handler);
  irq_set_exclusive_handler(PIO1_IRQ_0, __pio1_irq_0_handler);
  for (int i = 0; i < KALUMA_PIO_NUM; i++) {
    __pio_call_back[i] = jerry_undefined();
  }

  // pio module exports
  jerry_value_t exports = jerry_object();
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_ADD_PROGRAM,
                                pio_add_program_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_INIT, pio_sm_init_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_SET_ENABLED,
                                pio_sm_set_enabled_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_RESTART,
                                pio_sm_restart_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_EXEC, pio_sm_exec_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_PUT, pio_sm_put_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_GET, pio_sm_get_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_SET_PINS,
                                pio_sm_set_pins_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_RXFIFO,
                                pio_sm_rxfifo_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_TXFIFO,
                                pio_sm_txfifo_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_CLEAR_FIFOS,
                                pio_sm_clear_fifos_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_DRAIN_TXFIFO,
                                pio_sm_drain_txfifo_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_IRQ, pio_sm_irq_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_DORMANT, dormant_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_RANDOM_BIG_INT, random_big_int_fn);
  return exports;
}
