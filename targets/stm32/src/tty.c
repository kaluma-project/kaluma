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
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"
#include "stm32f4xx_it.h"
#include "system.h"
#include "usbd_cdc_if.h"

#define TTY_TX_RINGBUFFER_SIZE 1024
#define TTY_RX_RINGBUFFER_SIZE 2048
#define TTY_MAX_STRING_BUFFER_SIZE 1024

static unsigned char tty_tx_buffer[TTY_TX_RINGBUFFER_SIZE];
static unsigned char tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t tty_tx_ringbuffer;
static ringbuffer_t tty_rx_ringbuffer;

/**
 * this function is called in the pendable interrupt service routine which has
 * lowest priority to allow other interrupts service.
 */
void tty_transmit_data() {
  /* if the previous data is under transmitting, just return to avoid blocking
   */
  if (CDC_Transmit_IsReady()) {
    uint32_t len = ringbuffer_length(&tty_tx_ringbuffer);
    if (len) {
      uint8_t buf[TTY_TX_RINGBUFFER_SIZE];
      ringbuffer_read(&tty_tx_ringbuffer, buf, len);
      CDC_Transmit_FS(buf, len);
    }
  }
}

uint32_t tty_get_tx_data_length() {
  return ringbuffer_length(&tty_tx_ringbuffer);
}

uint32_t tty_get_bytes(uint8_t *buf, uint32_t nToRead) {
  __set_PRIMASK(1);

  uint32_t len = ringbuffer_length(&tty_rx_ringbuffer);
  if (len < nToRead) {
    nToRead = len;
  }

  __set_PRIMASK(0);
  ringbuffer_read(&tty_rx_ringbuffer, buf, nToRead);
  return nToRead;
}

uint32_t tty_fill_rx_bytes(uint8_t *buf, uint32_t nToWrite) {
  __set_PRIMASK(1);
  uint32_t space = ringbuffer_freespace(&tty_rx_ringbuffer);
  if (space < nToWrite) {
    nToWrite = space;
  }
  __set_PRIMASK(0);

  ringbuffer_write(&tty_rx_ringbuffer, buf, nToWrite);
  return nToWrite;
}

uint32_t tty_put_bytes(uint8_t *buf, uint32_t nToWrite) {
  __set_PRIMASK(1);
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space < nToWrite) {
    nToWrite = space;
  }
  __set_PRIMASK(0);

  ringbuffer_write(&tty_tx_ringbuffer, buf, nToWrite);
  return nToWrite;
}

void km_tty_init() {
  ringbuffer_init(&tty_tx_ringbuffer, tty_tx_buffer, sizeof(tty_tx_buffer));
  ringbuffer_init(&tty_rx_ringbuffer, tty_rx_buffer, sizeof(tty_rx_buffer));
}

uint32_t km_tty_available() { return ringbuffer_length(&tty_rx_ringbuffer); }

uint32_t km_tty_read(uint8_t *buf, size_t len) {
  if (km_tty_available() >= len) {
    ringbuffer_read(&tty_rx_ringbuffer, buf, len);
    return len;
  } else {
    return 0;
  }
}

uint32_t km_tty_read_sync(uint8_t *buf, size_t len, uint32_t timeout) {
  uint32_t sz;
  uint64_t current = km_gettime();
  uint64_t due = current + timeout;
  do {
    sz = km_tty_available();
    current = km_gettime();
  } while (current < due && sz < len);
  if (sz >= len) {
    ringbuffer_read(&tty_rx_ringbuffer, buf, len);
    return len;
  } else {
    return 0;
  }
}

uint8_t km_tty_getc() {
  uint8_t c = 0;
  if (km_tty_available()) {
    ringbuffer_read(&tty_rx_ringbuffer, &c, 1);
  }
  return c;
}

void km_tty_putc(char ch) {
  /* (ring)buffering the string instead of transmitting it via usb channel */
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space > 1) {
    ringbuffer_write(&tty_tx_ringbuffer, (uint8_t *)&ch, 1);
  }
  SetPendSV();
}

/**
 * Print formatted string to TTY
 */
void km_tty_printf(const char *fmt, ...) {
  va_list ap;
  char string[TTY_MAX_STRING_BUFFER_SIZE];

  va_start(ap, fmt);
  vsprintf(string, fmt, ap);
  va_end(ap);

  /* (ring)buffering the string instead of transmitting it via usb channel */
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space > strlen(string)) {
    tty_put_bytes((uint8_t *)string, strlen(string));
  }
  SetPendSV();
}
