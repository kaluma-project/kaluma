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

void km_pio_init(void) { return; }
static struct km_pio_s {
  uint8_t enabled;
  uint sm;
} km_pio[PIO_NUM];

static struct pio_program pio_code = {
    .origin = -1,
};

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

int km_pio_setup(uint8_t port, uint16_t *code, uint8_t code_length,
                 uint8_t pin_out) {
  PIO pio = __get_pio(port);
  if (pio == NULL) {
    return KM_PIO_ERROR;
  }

  pio_code.instructions = code;
  pio_code.length = code_length;
  uint offset = pio_add_program(pio, &pio_code);
  uint sm = pio_claim_unused_sm(pio, true);
  km_pio[port].sm = sm;
  pio_sm_config c = pio_get_default_sm_config();
  sm_config_set_wrap(&c, offset, offset + code_length - 1);

  // output settings
  if (pin_out < GPIO_NUM) {
    // parameter to this function.
    sm_config_set_out_pins(&c, pin_out, 1);
    // Set this pin's GPIO function (connect PIO to the pad)
    pio_gpio_init(pio, pin_out);
    // Set the pin direction to output at the PIO
    pio_sm_set_consecutive_pindirs(pio, sm, pin_out, 1, true);
  }
  pio_sm_init(pio, sm, offset, &c);
  // Set the state machine running
  pio_sm_set_enabled(pio, sm, true);
  return 0;
}

int km_pio_close(uint8_t port) {
  (void)port;
  return 0;
}

int km_pio_put_fifo(uint8_t port, uint32_t data) {
  PIO pio = __get_pio(port);
  if (pio == NULL) {
    return KM_PIO_ERROR;
  }
  pio_sm_put_blocking(pio, km_pio[port].sm, data);
  return 0;
}

uint32_t km_pio_get_fifo(uint8_t port) {
  PIO pio = __get_pio(port);
  if (pio == NULL) {
    return KM_PIO_ERROR;
  }
  return pio_sm_get_blocking(pio, km_pio[port].sm);
}
