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

#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>

typedef enum {
  SPI_SETUP_MODE_MASTER,
  SPI_SETUP_MODE_SLAVE
} spi_mode_t;

typedef enum {
  SPI_BITORDER_MSB,
  SPI_BITORDER_LSB
} spi_bitorder_t;

typedef enum {
  SPI_CLOCK_POLARITY_LOW,
  SPI_CLOCK_POLARITY_HIGH,
} spi_clock_polarity_t;

typedef enum {
  SPI_CLOCK_PHASE_1_EDGE,
  SPI_CLOCK_PHASE_2_EDGE,
} spi_clock_phase_t;

/**
 * Setup SPI bus
 * 
 * @param {uint8_t} bus
 * @param {spi_mode_t} mode
 * @param {uint8_t} cs_pin
 * @param {uint32_t} baudrate
 * @param {spi_clock_polarity_t} polarity
 * @param {spi_clock_phase_t} phase
 * @param {uint8_t} bits
 * @param {uint8_t} bit_order
 * @return result status code
 */
int spi_setup(uint8_t bus, spi_mode_t mode, uint8_t cs_pin, uint32_t baudrate,
  spi_clock_polarity_t polarity, spi_clock_phase_t phase, uint8_t bits, spi_bitorder_t bit_order);


/**
 * Send and receive data simultaneously to the SPI bus
 * 
 * @param {uint8_t} bus
 * @param {uint8_t*} tx_buf
 * @param {uint8_t*} rx_buf
 * @param {uint32_t} len
 * @param {uint32_t} timeout
 * @return the number of bytes read or -1 on timeout or nothing written.
 */
int spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, uint32_t timeout);

/**
 * Send data to the SPI bus
 * 
 * @param {uint8_t} bus
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @param {uint32_t} timeout
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int spi_send(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout);

/**
 * Receive data from the SPI bus and store them into a given buffer.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @param {uint32_t} timeout
 * @return the number of bytes read
 */
int spi_recv(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout);

/**
 * Close the SPI bus
 */
int spi_close(uint8_t bus);

#endif /* __SPI_H */
