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

#ifndef __TTY_LOW_LEVEL_H
#define __TTY_LOW_LEVEL_H

#include "buffer.h"

void tty_transmit_data(void);
void tty_init_ringbuffer(void);
uint32_t tty_get_rx_data_length(void);
uint32_t tty_get_tx_data_length(void);
uint8_t tty_get_byte(void);
uint32_t tty_get_bytes(uint8_t * buf, uint32_t nToRead);
uint32_t tty_fill_rx_bytes(uint8_t * buf, uint32_t nToWrite);
void tty_put_byte(uint8_t c);
uint32_t tty_put_bytes(uint8_t * buf, uint32_t nToWrite);
uint32_t tty_get_tx_freespace(void);
uint32_t tty_get_rx_freespace(void);

#endif /* __TTY_LOW_LEVEL_H */
