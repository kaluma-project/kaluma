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

#ifndef __KM_PIO_H
#define __KM_PIO_H

#include <stddef.h>
#include <stdint.h>

#define KM_PIO_ERROR -1

#define KM_PIO_FIFO_LOW 0
#define KM_PIO_FIFO_HIGH 1

/**
 * Initialize all PIO when system started
 */
void km_pio_init(void);

/**
 * Setup a PIO port.
 *
 * @param port port number of PIO block
 * @param code code array for PIO
 * @param code_length Length of the code.
 * @param pin_out output pin number for the PIO
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_setup(uint8_t port, uint16_t *code, uint8_t code_length,
                 uint8_t pin_out);

/**
 * Close the PIO port
 *
 * @param port port number of PIO block
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_close(uint8_t port);

/**
 * Put data into the PIO FIFO
 *
 * @param port port number of PIO block
 * @param data 32bit data value
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_put_fifo(uint8_t port, uint32_t data);

/**
 *  GEt data from the PIO FIFO
 *
 * @param port port number of PIO block
 * @return 32bit data value
 */
uint32_t km_pio_get_fifo(uint8_t port);
#endif /* __KM_PIO_H */
