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

#include "system.h"
#include "i2c.h"
#include "tty.h"

/**
 * Initialize all I2C when system started
 */
void i2c_init() {
}

/**
 * Cleanup all I2C when system cleanup
 */
void i2c_cleanup() {
}

int i2c_setup_master(uint8_t bus, uint32_t speed) {
  return 0;
}

int i2c_setup_slave(uint8_t bus, uint8_t address) {
  return 0;
}

int i2c_memWrite_master(uint8_t bus, uint8_t address, uint16_t memAddress, uint8_t memAdd16bit, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_memRead_master(uint8_t bus, uint8_t address, uint16_t memAddress, uint8_t memAdd16bit, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_write_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_write_slave(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_read_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_read_slave(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  return 0;
}

int i2c_close(uint8_t bus) {
  return 0;
}
