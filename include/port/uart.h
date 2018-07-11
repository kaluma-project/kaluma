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

/**
 * Setup a UART port. This have to manage an internal read buffer.
 * 
 * @param {uint8_t} port
 * @param {uint32_t} baudrate
 * @param {uint32_t} bits
 * @param {uint32_t} parity
 * @param {uint32_t} stop
 * @param {uint32_t} flow
 * @param {size_t} buffer_size The size of read buffer
 * @return {int} Positive number if successfully setup, negative otherwise.
 */
int uart_setup(uint8_t port, uint32_t baudrate, uint32_t bits,
  uint32_t parity, uint32_t stop, uint32_t flow, size_t buffer_size);

/**
 * Write a character to the port.
 * 
 * @param {uint8_t} port
 * @param {uint8_t} ch
 * @return {int} the number of bytes written or -1 if nothing written.
 */
int uart_write_char(uint8_t port, uint8_t ch);

/**
 * Write a given buffer to the port.
 * 
 * @param {uint8_t} port
 * @param {uint8_t*} buf
 * @param {size_t} len
 * @return {int} the number of bytes written or -1 if nothing written.
 */
int uart_write(uint8_t port, uint8_t *buf, size_t len);

/**
 * Check the number of bytes available to read.
 * 
 * @param {uint8_t} port
 * @return {int} the number of bytes in read buffer.
 */
uint32_t uart_available(uint8_t port);

/**
 * Look a character at the index in read buffer.
 * 
 * @param {uint8_t} port
 * @param {uint32_t} index
 * @return {uint8_t} a character at the index in read buffer.
 */
uint8_t uart_available_at(uint8_t port, uint32_t index);

/**
 * Read a character from the port.
 * 
 * @param {uint8_t} port
 * @return {int} a character read or -1
 */
int uart_read_char(uint8_t port);

/**
 * Read bytes from the port and store them into a given buffer.
 * 
 * @param {uint8_t} port
 * @param {uint8_t*} buf
 * @param {size_t} len
 * @return {int} the number of bytes read
 */
uint32_t uart_read(uint8_t port, uint8_t *buf, size_t len);

/**
 * Close the UART port
 * 
 * @param {uint8_t} port
 */
int uart_close(uint8_t port);

#endif /* __UART_H */


