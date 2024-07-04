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

#ifndef __KM_SPI_H
#define __KM_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  KM_SPI_MODE_0,  // (CPOL=0/CPHA=0)
  KM_SPI_MODE_1,  // (CPOL=0/CPHA=1)
  KM_SPI_MODE_2,  // (CPOL=1/CPHA=0)
  KM_SPI_MODE_3   // (CPOL=1/CPHA=1)
} km_spi_mode_t;

typedef enum { KM_SPI_BITORDER_MSB, KM_SPI_BITORDER_LSB } km_spi_bitorder_t;

typedef struct {
  int8_t miso;
  int8_t mosi;
  int8_t sck;
} km_spi_pins_t;

/**
 * Return default UART pins. -1 means there is no default value on that pin.
 */
km_spi_pins_t km_spi_get_default_pins(uint8_t bus);
/**
 * Initialize all SPI when system started
 */
void km_spi_init();

/**
 * Cleanup all SPI when system cleanup
 */
void km_spi_cleanup();

/**
 * Setup SPI bus as the master device
 *
 * @param bus The bus number.
 * @param mode SPI mode of clock polarity and phase.
 * @param baudrate Baud rate.
 * @param bit_order Bit order (MSB or LSB).
 * @param bits Number of bits in each transferred word.
 * @param pins pin numbers for the SCK/MISO/MOSI
 * @param miso_pullup true when MISO internal pull up is needed.
 * @return Returns 0 on success or minus value (err) on failure.
 */
int km_spi_setup(uint8_t bus, km_spi_mode_t mode, uint32_t baudrate,
                 km_spi_bitorder_t bitorder, km_spi_pins_t pins,
                 bool miso_pullup);

/**
 * Send and receive data simultaneously to the SPI bus
 *
 * @param bus
 * @param tx_buf
 * @param rx_buf
 * @param len
 * @param timeout
 * @return the number of bytes read or minus value (err) on timeout or nothing
 * written.
 */
int km_spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, size_t len,
                    uint32_t timeout);

/**
 * Send data to the SPI bus
 *
 * @param bus
 * @param buf
 * @param len
 * @param timeout
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int km_spi_send(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout);

/**
 * Receive data from the SPI bus and store them into a given buffer.
 *
 * @param {uint8_t} bus
 * @param {uint8_t} send_byte byte to send
 * @param {uint8_t*} buf
 * @param {size_t} len
 * @param {uint32_t} timeout
 * @return {int} the number of bytes read
 */
int km_spi_recv(uint8_t bus, uint8_t send_byte, uint8_t *buf, size_t len,
                uint32_t timeout);

/**
 * Set SPI baudrate - change the clock frequency
 *
 * @param bus The bus number.
 * @param baudrate Baud rate.
 * @return int Returns 0 on success or minus value (err) on failure.
 */
int km_set_spi_baudrate(uint8_t bus, uint32_t baudrate);

/**
 * Close the SPI bus
 */
int km_spi_close(uint8_t bus);

#ifdef __cplusplus
}
#endif

#endif /* __KM_SPI_H */
