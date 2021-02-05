/* Copyright (c) 2017 Kalamu
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
#include "ringbuffer.h"

void tty_init() {
}

uint32_t tty_available() {
  return 0;
}

uint32_t tty_read(uint8_t *buf, size_t len) {
  return 0;
}

uint32_t tty_read_sync(uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}


uint8_t tty_getc() {
  return 0;
}

void tty_putc(char ch) {
  putchar(ch);
}

/**
 * Print formatted string to TTY
 */
void tty_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
