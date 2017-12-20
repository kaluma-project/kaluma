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

#include "stm32f4xx.h"
#include "uart.h"
#include "uart_low_level.h"

static uint32_t uart_time_out = (uint32_t)-1;

/** 
*/
int uart_open(uint8_t bus, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, uint32_t timeout) {
  uart_time_out = timeout;
  uart_init(bus, baudrate, bits, parity, stop, flow);
}

/** 
*/
int uart_write_char(uint8_t bus, uint8_t ch) {
  int len=1;
  uart_status_t status = uart_transmit(bus, &ch, 1, uart_time_out);

  if (status != UART_STATUS_SUCCESS) {
    len = -1;
  }
  return len;
}

/** 
*/
int uart_write(uint8_t bus, uint8_t *buf, uint32_t len) {
  uart_status_t status = uart_transmit(bus, buf, len, uart_time_out);

  if (status != UART_STATUS_SUCCESS) {
    len = -1;
  }
  return len;
}

/** 
*/
int uart_read_char(uint8_t bus) {
  int ch=0;
  uart_status_t status = uart_receive(bus, (uint8_t *)&ch, 1, uart_time_out);

  if (status != UART_STATUS_SUCCESS) {
    ch = -1;
  } else {
    ch = ch & 0x0ff;
  }
  
  return ch;
}

/** 
*/
int uart_read(uint8_t bus, uint8_t *buf, uint32_t len) {
  uart_status_t status = uart_receive(bus, buf, len, uart_time_out);

  if (status == UART_STATUS_SUCCESS) {
    return len;
  } else {
    return -1;
  }
}

/** 
*/
void uart_close(uint8_t bus) {
  uart_deinit(bus);
}


//
//
//
void uart_test()
{
  int d;
  uint8_t bus = 0;
  uint8_t buf[5];

  uart_open(bus, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1, UART_HWCONTROL_NONE, (uint32_t)1000);

  while(1)
  {
    d = uart_read_char(bus);
    if (d != -1) {
      uart_write_char(bus, (uint8_t)d);
    }
    else {
      tty_printf("timeout-1 \r\n");
    }

    d = uart_read(bus, buf, 1);
    if(d != -1 ) {
      uart_write(bus, buf, 1);
    }
    else {
      tty_printf("timeout-2 \r\n");
    }
  }
  uart_close(bus);
}