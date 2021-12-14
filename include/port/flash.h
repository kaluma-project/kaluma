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

#ifndef __KM_FLASH_H
#define __KM_FLASH_H

#include <stdint.h>
#include <stdio.h>

extern const uint8_t *km_flash_addr;

/**
 * @brief Program data to internal flash
 *
 * @param sector sector number to program
 * @param offset offset to the sector (multiple of KALUMA_FLASH_PAGE_SIZE)
 * @param buffer buffer to write
 * @param size size of buffer to write (multiple of KALUMA_FLASH_PAGE_SIZE)
 * @return negative on error
 */
int km_flash_program(uint32_t sector, uint32_t offset, uint8_t *buffer,
                     size_t size);

/**
 * @brief Erase data in internal flash
 *
 * @param sector sector number to erase
 * @param count how many sectors to erase from the sector number
 * @return negative on error
 */
int km_flash_erase(uint32_t sector, size_t count);

#endif /* __KM_FLASH_H */
