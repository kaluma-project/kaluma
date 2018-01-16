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

#ifndef __UART_LOW_LEVEL_H
#define __UART_LOW_LEVEL_H

#include "buffer.h"

int uart_init_ringbuffer(uint8_t bus, uint32_t size);
void uart_deinit_ringbuffer(uint8_t bus);
uint8_t uart_read_char_ringbuffer(uint8_t bus);
uint32_t uart_read_ringbuffer(uint8_t bus, uint8_t * buf, uint32_t len);
uint32_t uart_available_ringbuffer(uint8_t bus);
void uart_fill_ringbuffer(uint8_t bus, uint8_t ch);

#endif /* __UART_LOW_LEVEL_H */
