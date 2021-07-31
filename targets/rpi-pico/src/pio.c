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
#include "pio.h"

#include <stdlib.h>

#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "ringbuffer.h"
#include "rpi_pico.h"

static struct km_pio_s {
  uint8_t enabled;
  uint8_t code_length;
  uint offset;
} km_pio[PIO_NUM];

static struct pio_program pio_code = {
    .origin = -1,
};

static void __reset_pio_block(void) {
  pio_clear_instruction_memory(pio0);
  pio_clear_instruction_memory(pio1);
  for (int i = 0; i < PIO_NUM; i++) {
    km_pio[i].enabled = KM_PIO_PORT_DISABLE;
    km_pio[i].code_length = 0;
    km_pio[i].offset = 0;
  }
}
void km_pio_init(void) { __reset_pio_block(); }

void km_pio_cleanup(void) { __reset_pio_block(); }

static PIO __get_pio(uint8_t port) {
  PIO pio;
  if (port == 0) {
    pio = pio0;
  } else if (port == 1) {
    pio = pio1;
  } else {
    return NULL;
  }
  return pio;
}

static bool __sm_enabled(uint8_t port, uint8_t sm) {
  if (km_pio[port].enabled & ((0x10) << sm)) {
    return true;
  }
  return false;
}

int km_pio_port_init(uint8_t port, uint16_t *code, uint8_t code_length) {
  PIO pio = __get_pio(port);
  if ((pio == NULL) || (km_pio[port].enabled == KM_PIO_PORT_ENABLE)) {
    return KM_PIO_ERROR;
  }
  km_pio_close(port);
  pio_code.instructions = code;
  pio_code.length = code_length;
  km_pio[port].offset = pio_add_program(pio, &pio_code);
  km_pio[port].code_length = code_length;
  km_pio[port].enabled = KM_PIO_PORT_ENABLE;  // Port is enabled
  return 0;
}

int km_pio_sm_setup(uint8_t port, uint8_t sm, uint8_t pin_out,
                    uint8_t pin_mode) {
  (void)pin_mode;  // OUTPUT is the only option to use now.
  PIO pio = __get_pio(port);
  if ((pio == NULL) || (km_pio[port].enabled != KM_PIO_PORT_ENABLE) ||
      (__sm_enabled(port, sm))) {
    return KM_PIO_ERROR;
  }

  pio_sm_config c = pio_get_default_sm_config();
  sm_config_set_wrap(&c, km_pio[port].offset,
                     km_pio[port].offset + km_pio[port].code_length - 1);

  // output settings
  if (pin_out < GPIO_NUM) {
    // parameter to this function.
    sm_config_set_out_pins(&c, pin_out, 1);
    // Set this pin's GPIO function (connect PIO to the pad)
    pio_gpio_init(pio, pin_out);
    // Set the pin direction to output at the PIO
    pio_sm_set_consecutive_pindirs(pio, sm, pin_out, 1, true);
  }
  pio_sm_init(pio, sm, km_pio[port].offset, &c);
  // Set the state machine running
  pio_sm_set_enabled(pio, sm, true);
  return 0;
}

int km_pio_close(uint8_t port) {
  PIO pio = __get_pio(port);
  if ((pio == NULL) || !(km_pio[port].enabled & KM_PIO_PORT_ENABLE)) {
    return KM_PIO_ERROR;
  }
  for (int i = 0; i < KM_PIO_NO_SM; i++) {
    pio_sm_unclaim(pio, i);
  }
  pio_clear_instruction_memory(pio);
  return 0;
}

int km_pio_put_fifo(uint8_t port, uint8_t sm, uint32_t data) {
  PIO pio = __get_pio(port);
  if ((pio == NULL) || !(km_pio[port].enabled & KM_PIO_PORT_ENABLE)) {
    return KM_PIO_ERROR;
  }
  pio_sm_put_blocking(pio, sm, data);
  return 0;
}

uint32_t km_pio_get_fifo(uint8_t port, uint8_t sm) {
  PIO pio = __get_pio(port);
  if ((pio == NULL) || !(km_pio[port].enabled & KM_PIO_PORT_ENABLE)) {
    return KM_PIO_ERROR;
  }
  return pio_sm_get_blocking(pio, sm);
}
