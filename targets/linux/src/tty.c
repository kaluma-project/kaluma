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

#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ringbuffer.h"
#include "system.h"

#define TTY_RX_RINGBUFFER_SIZE 2048

static unsigned char __tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t __tty_rx_ringbuffer;

void km_tty_init() {
  ringbuffer_init(&__tty_rx_ringbuffer, __tty_rx_buffer,
                  sizeof(__tty_rx_buffer));

  int fd = STDIN_FILENO;  // stdin
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

uint32_t km_tty_available() {
  unsigned char ch[1];
  int sz = read(STDIN_FILENO, ch, 1);
  // printf("sz=%d\r\n", sz);
  while (sz > 0) {
    ringbuffer_write(&__tty_rx_ringbuffer, ch, 1);
    sz = read(STDIN_FILENO, ch, 1);
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
  uint64_t to = km_gettime() + timeout;
  do {
    sz = km_tty_available();
  } while (km_gettime() < to && sz < len);
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

void km_tty_putc(char ch) { printf("%c", ch); }

/**
 * Print formatted string to TTY
 */
void km_tty_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
