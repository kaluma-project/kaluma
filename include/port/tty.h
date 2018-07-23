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

#ifndef __TTY_H
#define __TTY_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * Initialize TTY
 */
void tty_init();

/**
 * Check the number of bytes available to read.
 * 
 * @return the number of bytes in TTY read buffer.
 */
uint32_t tty_available();

/**
 * Read bytes from TTY read buffer.
 * 
 * @param buf
 * @param len
 * @return the number of bytes read
 */
uint32_t tty_read(uint8_t *buf, size_t len);

/**
 * Read a char from TTY
 * 
 * @return char
 */
uint8_t tty_getc();

/**
 * Write a char to TTY
 * 
 * @param ch a character to write
 */
void tty_putc(char ch);

/**
 * Write a formatted string to TTY
 * 
 * @param fmt a string format
 * @param ... arguments for the format
 */
void tty_printf(const char *fmt, ...);

#endif /* __TTY_H */
