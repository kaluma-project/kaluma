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

#include "spi.h"
#include "gpio.h"

/**
 * Return default SPI pins. -1 means there is no default value on that pin.
 */
km_spi_pins_t km_spi_get_default_pins(uint8_t bus) {
  km_spi_pins_t pins;
  pins.miso = 0;
  pins.mosi = 1;
  pins.clk = 2;
  return pins;
}
/**
 * Initialize all SPI when system started
 */
void km_spi_init() {
}

/**
 * Cleanup all SPI when system cleanup
 */
void km_spi_cleanup() {
}

/** SPI Setup
*/
int km_spi_setup(uint8_t bus, km_spi_mode_t mode, uint32_t baudrate, km_spi_bitorder_t bitorder, km_spi_pins_t pins) {
  return 0;
}

int km_spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout) {
  return 0;
}

int km_spi_send(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int km_spi_recv(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int km_spi_close(uint8_t bus) {
  return 0;
}
