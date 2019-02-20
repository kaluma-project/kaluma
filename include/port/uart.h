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

typedef enum {
  UART_PARITY_TYPE_NONE = 0,
  UART_PARITY_TYPE_ODD,
  UART_PARITY_TYPE_EVEN
} uart_parity_type_t;

typedef enum {
  UART_FLOW_NONE = 0,
  UART_FLOW_RTS,
  UART_FLOW_CTS,
  UART_FLOW_RTS_CTS
} uart_flow_control_t;

/**
 * Setup a UART port. This have to manage an internal read buffer.
 *
 * @param port
 * @param baudrate
 * @param bits databits 8 or 9
 * @param parity
 * @param stop stopbits 1 or 2
 * @param flow
 * @param buffer_size The size of read buffer
 * @return Positive number if successfully setup, negative otherwise.
 */
int uart_setup(uint8_t port, uint32_t baudrate, uint8_t bits,
  uart_parity_type_t parity, uint8_t stop, uart_flow_control_t flow,
  size_t buffer_size);

/**
 * Write a given buffer to the port.
 *
 * @param port
 * @param buf
 * @param len
 * @return the number of bytes written or -1 if nothing written.
 */
int uart_write(uint8_t port, uint8_t *buf, size_t len);

/**
 * Check the number of bytes available to read.
 *
 * @param port
 * @return the number of bytes in read buffer.
 */
uint32_t uart_available(uint8_t port);

/**
 * Look a character at the offset in read buffer.
 *
 * @param port
 * @param offset
 * @return a character at the offset in read buffer.
 */
uint8_t uart_available_at(uint8_t port, uint32_t offset);

/**
 * Get the size of read buffer.
 *
 * @param port
 * @return size of read buffer.
 */
uint32_t uart_buffer_size(uint8_t port);

/**
 * Read bytes from the port and store them into a given buffer.
 *
 * @param port
 * @param buf
 * @param len
 * @return the number of bytes read
 */
uint32_t uart_read(uint8_t port, uint8_t *buf, size_t len);

/**
 * Close the UART port
 *
 * @param port
 */
int uart_close(uint8_t port);

#endif /* __UART_H */
