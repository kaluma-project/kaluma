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
#include "tty.h"

#include <stdarg.h>

#include "hardware/timer.h"
#include "pico/stdlib.h"
#include "repl.h"
#include "ringbuffer.h"
#include "runtime.h"
#include "system.h"
#include "tusb.h"

#define TTY_RX_RINGBUFFER_SIZE 2048
#define ETX 0x03  // Ctrl + C, SIGINT
static unsigned char __tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t __tty_rx_ringbuffer;

void km_tty_init() {
  ringbuffer_init(&__tty_rx_ringbuffer, __tty_rx_buffer,
                  sizeof(__tty_rx_buffer));
  tud_cdc_set_wanted_char(ETX);  // Crtl + C
}

TU_ATTR_WEAK void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  if (wanted_char == ETX) {
    km_repl_state_t *state = km_get_repl_state();
    if (state->ymodem_state == 0) {
      km_runtime_set_vm_stop(1);
      tud_cdc_read_flush();  // flush read fifo
    }
  }
}

uint32_t km_tty_available() {
  int ch = getchar_timeout_us(0);
  while (ch >= 0) {
    ringbuffer_write(&__tty_rx_ringbuffer, (uint8_t *)&ch, 1);
    ch = getchar_timeout_us(0);
    km_runtime_set_vm_stop(0);
  }
  return ringbuffer_length(&__tty_rx_ringbuffer);
}

uint32_t km_tty_read(uint8_t *buf, size_t len) {
  if (km_tty_available() >= len) {
    ringbuffer_read(&__tty_rx_ringbuffer, buf, len);
    return len;
  } else {
    return 0;
  }
}

uint32_t km_tty_read_sync(uint8_t *buf, size_t len, uint32_t timeout) {
  uint32_t sz;
  absolute_time_t timeout_ms = delayed_by_ms(get_absolute_time(), timeout);
  do {
    sz = km_tty_available();
#ifdef NDEBUG
  } while ((get_absolute_time() < timeout_ms) && (sz < len));
#else
  } while ((get_absolute_time()._private_us_since_boot < timeout_ms._private_us_since_boot) && (sz < len));
#endif
  if (sz >= len) {
    ringbuffer_read(&__tty_rx_ringbuffer, buf, len);
    return len;
  } else {
    return 0;
  }
}

uint8_t km_tty_getc() {
  uint8_t c = 0;
  if (km_tty_available()) {
    ringbuffer_read(&__tty_rx_ringbuffer, &c, 1);
  }
  return c;
}

void km_tty_putc(char ch) { putchar(ch); }

/**
 * Print formatted string to TTY
 */
void km_tty_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
