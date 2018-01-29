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

#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

typedef enum {
  I2C_MODE_MASTER,
  I2C_MODE_SLAVE
}i2c_mode_t;

/**
 * Setup a I2C bus as master
 * 
 * @param {uint8_t} bus
 * @return {int} result status code
 */
int i2c_setup_master(uint8_t bus);

/**
 * Setup a I2C bus as slave
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @return {int} result status code
 */
int i2c_setup_slave(uint8_t bus, uint8_t address);

/**
 * Write a given buffer to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t*} buf
 * @param {size_t} len
 * @param {uint32_t} timeout
 * @return {int} the number of bytes written or -1 on timeout or nothing written.
 */
int i2c_write(uint8_t bus, uint8_t address, uint8_t *buf, size_t len,
  uint32_t timeout);

/**
 * Write a given buffer to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t} ch
 * @param {uint32_t} timeout
 * @return {int} the number of bytes written or -1 on timeout or nothing written.
 */
int i2c_write_char(uint8_t bus, uint8_t address, uint8_t ch, uint32_t timeout);

/**
 * Read bytes from the bus and store them into a given buffer.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t*} buf
 * @param {size_t} len
 * @return {int} the number of bytes read
 */
int i2c_read(uint8_t bus, uint8_t address, uint8_t *buf, size_t len,
  uint32_t timeout);

/**
 * Read a character from the bus.
 * 
 * @return {int} a character read
 */
int i2c_read_char(uint8_t bus, uint8_t address, uint32_t timeout);

/**
 * Close the I2C bus
 * 
 * @return {int}
 */
int i2c_close(uint8_t bus);

#endif /* __I2C_H */

