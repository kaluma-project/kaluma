/* Copyright (c) 2017 Kameleon
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
#include "usbd_cdc_if.h"
#include "ringbuffer.h"

#define TTY_TX_RINGBUFFER_SIZE 1024
#define TTY_RX_RINGBUFFER_SIZE 1024

static unsigned char tty_tx_buffer[TTY_TX_RINGBUFFER_SIZE];
static unsigned char tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t tty_tx_ringbuffer;
static ringbuffer_t tty_rx_ringbuffer;


/**
 * this function is called in the pendable interrupt service routine which has 
 * lowest priority to allow other interrupts service.
 */
void tty_transmit_data() {   
  /* if the previous data is under transmitting, just return to avoid blocking */
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


uint32_t tty_get_bytes(uint8_t * buf, uint32_t nToRead) {
  __set_PRIMASK(1);
  
  uint32_t len = ringbuffer_length(&tty_rx_ringbuffer);
  if (len < nToRead) {
    nToRead = len;
  }
  
  __set_PRIMASK(0);
  ringbuffer_read(&tty_rx_ringbuffer, buf, nToRead);
  return nToRead;
}

uint32_t tty_fill_rx_bytes(uint8_t * buf, uint32_t nToWrite) {
  __set_PRIMASK(1);
  uint32_t space = ringbuffer_freespace(&tty_rx_ringbuffer);
  if (space < nToWrite) {
    nToWrite = space;
  }
  __set_PRIMASK(0);
  
  ringbuffer_write(&tty_rx_ringbuffer, buf, nToWrite);
  return nToWrite;
}


uint32_t tty_put_bytes(uint8_t * buf, uint32_t nToWrite) {
  __set_PRIMASK(1);  
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space < nToWrite) {
    nToWrite = space;
  }
  __set_PRIMASK(0);
  
  ringbuffer_write(&tty_tx_ringbuffer, buf, nToWrite);
  return nToWrite;
}

void tty_init() {
  ringbuffer_init(&tty_tx_ringbuffer, tty_tx_buffer, sizeof(tty_tx_buffer));
  ringbuffer_init(&tty_rx_ringbuffer, tty_rx_buffer, sizeof(tty_rx_buffer));
}

uint32_t tty_available() {
  return ringbuffer_length(&tty_rx_ringbuffer);
}

uint32_t tty_read(uint8_t *buf, size_t len) {
  if (tty_available()) {
    ringbuffer_read(&tty_rx_ringbuffer, buf, len);
  } 
}

uint8_t tty_getc() {
  uint8_t c = 0;
  if (tty_available()) {
    ringbuffer_read(&tty_rx_ringbuffer, &c, 1);
  }
  return c;
}

void tty_putc(char ch) {
  /* (ring)buffering the string instead of transmitting it via usb channel */
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space > 1) {
    ringbuffer_write(&tty_tx_ringbuffer, (uint8_t *)&ch, 1);
  }
  SetPendSV(); 
}


void tty_printf(const char *fmt, ...) {
  va_list ap;
  char string[256];
  
  va_start(ap,fmt);
  vsprintf(string, fmt, ap);   
  va_end(ap);
  
  /* (ring)buffering the string instead of transmitting it via usb channel */
  uint32_t space = ringbuffer_freespace(&tty_tx_ringbuffer);
  if (space > strlen(string)) {
    tty_put_bytes((uint8_t *)string, strlen(string));
  }
  SetPendSV();
}
