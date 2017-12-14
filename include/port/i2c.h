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

/**
 * Open a I2C bus as master
 * 
 * @param {uint8_t} bus
 * @return result status code
 */
int i2c_open_master(uint8_t bus);

/**
 * Open a I2C bus as slave
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @return result status code
 */
int i2c_open_slave(uint8_t bus, uint8_t address);

/**
 * Write a given buffer to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @param {uint32_t} timeout
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int i2c_write(uint8_t bus, uint8_t address, uint8_t *buf, uint32_t len, uint32_t timeout);

/**
 * Write a given buffer to the bus.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t} ch
 * @param {uint32_t} timeout
 * @return the number of bytes written or -1 on timeout or nothing written.
 */
int i2c_write_char(uint8_t bus, uint8_t address, uint8_t ch, uint32_t timeout);

/**
 * Read bytes from the bus and store them into a given buffer.
 * 
 * @param {uint8_t} bus
 * @param {uint8_t} address
 * @param {uint8_t*} buf
 * @param {uint32_t} len
 * @return the number of bytes read
 */
int i2c_read(uint8_t bus, uint8_t address, uint8_t *buf, uint32_t len);

/**
 * Read a character from the bus.
 * 
 * @return a character read
 */
uint8_t i2c_read_char(uint8_t bus, uint8_t address);

/**
 * Close the I2C bus
 */
void i2c_close(uint8_t bus);

#endif /* __I2C_H */
