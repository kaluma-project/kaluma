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

#include "hardware/irq.h"
#include "hardware/pio.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pico/stdlib.h"
#include "rp2_magic_strings.h"

static PIO __pio(uint8_t pio) {
  if (pio == 0) {
    return pio0;
  } else if (pio == 1) {
    return pio1;
  }
  return NULL;
}

JERRYXX_FUN(pio_add_program_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG(1, "prog");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t prog = JERRYXX_GET_ARG(1);
  pio_program_t pio_prog;
  if (jerry_value_is_typedarray(prog) &&
      jerry_get_typedarray_type(prog) ==
          JERRY_TYPEDARRAY_UINT16) { /* Uint16Array */
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t array_buffer =
        jerry_get_typedarray_buffer(prog, &byteOffset, &byteLength);
    pio_prog.origin = -1;
    pio_prog.instructions =
        (uint16_t *)jerry_get_arraybuffer_pointer(array_buffer),
    pio_prog.length = jerry_get_arraybuffer_byte_length(array_buffer) / 2,
    jerry_release_value(array_buffer);
    PIO _pio = __pio(pio);
    int offset = pio_add_program(_pio, &pio_prog);
    return jerry_create_number(offset);
  } else {
    return jerry_create_error(
        JERRY_ERROR_TYPE,
        (const jerry_char_t *)"The prog argument must be Uint16Array");
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
  // TODO: setup freq ...
  // setup in pins
  uint8_t in_base =
      (uint8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_IN_BASE, 0);
  sm_config_set_in_pins(&sm_config, in_base);
  pio_sm_set_consecutive_pindirs(_pio, sm, in_base, 1, false);
  pio_gpio_init(_pio, in_base);
  // setup out pins
  uint8_t out_base = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUT_BASE, 0);
  uint8_t out_count = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUT_COUNT, 0);
  sm_config_set_out_pins(&sm_config, out_base, out_count);
  pio_sm_set_consecutive_pindirs(_pio, sm, out_base, out_count, true);
  for (int i = 0; i < out_count; i++) {
    pio_gpio_init(_pio, out_base + i);
  }
  // setup set pins
  uint8_t set_base = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_SET_BASE, 0);
  uint8_t set_count = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_SET_COUNT, 0);
  sm_config_set_set_pins(&sm_config, set_base, set_count);
  // setup sideset pins
  bool sideset = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_SIDESET, false);
  if (sideset) {
    uint8_t sideset_base = (uint8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_SIDESET_BASE, 0);
    uint8_t sideset_bits = (uint8_t)jerryxx_get_property_number(
        options, MSTR_RP2_PIO_SM_SIDESET_BITS, 0);
    bool sideset_opt = (uint8_t)jerryxx_get_property_boolean(
        options, MSTR_RP2_PIO_SM_SIDESET_OPT, false);
    bool sideset_pindirs = (uint8_t)jerryxx_get_property_boolean(
        options, MSTR_RP2_PIO_SM_SIDESET_PINDIRS, false);
    sm_config_set_sideset_pins(&sm_config, sideset_base);
    sm_config_set_sideset(&sm_config, sideset_bits, sideset_opt,
                          sideset_pindirs);
  }
  // setup jmp pin
  uint8_t jmp_pin =
      (uint8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_JMP_PIN, 0);
  sm_config_set_jmp_pin(&sm_config, jmp_pin);
  // setup wrap
  uint8_t wrap_target = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_WRAP_TARGET, 0);
  uint8_t wrap =
      (uint8_t)jerryxx_get_property_number(options, MSTR_RP2_PIO_SM_WRAP, 31);
  sm_config_set_wrap(&sm_config, wrap_target, wrap);
  // setup in-shift
  uint8_t inshift_dir = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_INSHIFT_DIR, 0);
  bool autopush = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_AUTOPUSH, 0);
  uint8_t push_threshold = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_PUSH_THRESHOLD, 0);
  sm_config_set_in_shift(&sm_config, inshift_dir, autopush, push_threshold);
  // setup out-shift
  uint8_t outshift_dir = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUTSHIFT_DIR, 0);
  bool autopull = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_AUTOPULL, 0);
  uint8_t pull_threshold = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_PULL_THRESHOLD, 0);
  sm_config_set_out_shift(&sm_config, outshift_dir, autopull, pull_threshold);
  // setup fifoJoin
  uint8_t fifo_join = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_FIFO_JOIN, 0);
  sm_config_set_fifo_join(&sm_config, fifo_join);
  /*
  // TODO: setup out special
  bool out_sticky = (uint8_t)jerryxx_get_property_boolean(
      options, MSTR_RP2_PIO_SM_OUT_STICKY, false);
  int out_enable_pin = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_OUT_ENABLE_PIN, -1);
  sm_config_set_out_special(&sm_config, out_sticky, out_enable_pin > -1,
                            out_enable_pin);
  // TODO: setup mov status
  uint8_t move_status_sel = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_MOV_STATUS_SEL, 0);
  uint32_t move_status_n = (uint8_t)jerryxx_get_property_number(
      options, MSTR_RP2_PIO_SM_MOV_STATUS_N, 0);
  sm_config_set_mov_status(&sm_config, move_status_sel, move_status_n);
  */
  pio_sm_init(_pio, sm, wrap_target, &sm_config);
  return jerry_create_undefined();
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
  return jerry_create_undefined();
}

JERRYXX_FUN(pio_sm_restart_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  pio_sm_restart(_pio, sm);
  return jerry_create_undefined();
}

JERRYXX_FUN(pio_sm_exec_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_NUMBER(2, "inst");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t inst = (uint8_t)JERRYXX_GET_ARG_NUMBER(2);
  PIO _pio = __pio(pio);
  pio_sm_exec(_pio, sm, inst);
  return jerry_create_undefined();
}

JERRYXX_FUN(pio_sm_put_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  JERRYXX_CHECK_ARG_NUMBER(2, "data");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint32_t data = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  PIO _pio = __pio(pio);
  pio_sm_put_blocking(_pio, sm, data);
  return jerry_create_undefined();
}

JERRYXX_FUN(pio_sm_get_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "pio");
  JERRYXX_CHECK_ARG_NUMBER(1, "sm");
  uint8_t pio = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t sm = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  PIO _pio = __pio(pio);
  uint32_t data = pio_sm_get_blocking(_pio, sm);
  return jerry_create_number(data);
}

/**
 * Initialize 'rp2' module and return exports
 */
jerry_value_t module_rp2_init() {
  // clear PIO and state machines
  for (int i = 0; i < 4; i++) {
    pio_sm_unclaim(pio0, i);
    pio_sm_unclaim(pio1, i);
  }
  pio_clear_instruction_memory(pio0);
  pio_clear_instruction_memory(pio1);

  // pio module exports
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_ADD_PROGRAM,
                                pio_add_program_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_INIT, pio_sm_init_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_SET_ENABLED,
                                pio_sm_set_enabled_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_STATE_MACHINE_RESTART,
                                pio_sm_restart_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_STATE_MACHINE_EXEC,
                                pio_sm_exec_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_PUT, pio_sm_put_fn);
  jerryxx_set_property_function(exports, MSTR_RP2_PIO_SM_GET, pio_sm_get_fn);
  return exports;
}
