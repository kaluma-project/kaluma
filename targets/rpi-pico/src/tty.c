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
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tty.h"
#include "system.h"
#include "pico/stdlib.h"

#define KM_TTY_RX_BUFFER_SIZE   32
static struct __tty_rx_s {
  char buffer[KM_TTY_RX_BUFFER_SIZE];
  int length;
} __tty_rx;

void km_tty_init() {
  stdio_init_all();
  __tty_rx.length = 0;
  memset(__tty_rx.buffer, 0, KM_TTY_RX_BUFFER_SIZE);
}

uint32_t km_tty_available() {
  int ch = getchar_timeout_us(0);
  while(ch > 0)
  {
    __tty_rx.buffer[__tty_rx.length++] = (char)ch;
    ch = getchar_timeout_us(0);
  }
  return __tty_rx.length;
}

uint32_t km_tty_read(uint8_t *buf, size_t len) {
  if (__tty_rx.length >= len)
  {
    memcpy(buf, __tty_rx.buffer, len);
    __tty_rx.length -= len;
    return len;
  }
  return 0;
}

uint32_t km_tty_read_sync(uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}


uint8_t km_tty_getc() {
  return getchar_timeout_us(0);
}

void km_tty_putc(char ch) {
  printf("%c", ch);
}

/**
 * Print formatted string to TTY
 */
void km_tty_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
