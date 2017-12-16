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

#ifndef __UART_H
#define __UART_H

#include <stdint.h>


enum {
  UART_PARITY_TYPE_NONE,
  UART_PARITY_TYPE_ODD,
  UART_PARITY_TYPE_EVEN
};

enum {
  UART_FLOW_NONE,
  UART_FLOW_RTS,
  UART_FLOW_CTS,
  UART_FLOW_RTS_CTS
};

enum {
  UART_STOP_1_BIT,
  UART_STOP_2_BIT
};

enum {
  UART_DATA_8_BIT,
  UART_DATA_9_BIT
};

typedef enum {
  UART_STATUS_SUCCESS,
  UART_STATUS_FAIL,
  UART_STATUS_TIMEOUT
} uart_status_t;

/**
 * Open a UART bus
 * 
 * @param {uint8_t} bus
 * @param {uint32_t} baudrate
 * @param {uint32_t} bits
 * @param {uint32_t} parity
 * @param {uint32_t} stop
 * @param {uint32_t} flow
 * @param {uint32_t} timeout
 * @return
 */
int uart_open(uint8_t bus, uint32_t baudrate, uint32_t bits, 
  uint32_t parity, uint32_t stop, uint32_t flow, uint32_t timeout);

/**
 * Write a character to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} ch
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int uart_write_char(uint8_t bus, uint8_t ch);

/**
 * Write a given buffer to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int uart_write(uint8_t bus, uint8_t *buf, uint32_t len);

/**
 * Read a character from the bus.
 * 
 * @param {uint8_t} bus
 * @return a character read or -1 on timeout
 */
int uart_read_char(uint8_t bus);

/**
 * Read bytes from the bus and store them into a given buffer.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @return the number of bytes read or -1 on timeout
 */
int uart_read(uint8_t bus, uint8_t *buf, uint32_t len);

/**
 * Close the UART bus
 * 
 * @param {uint8_t} bus
 */
void uart_close(uint8_t bus);

#endif /* __UART_H */

