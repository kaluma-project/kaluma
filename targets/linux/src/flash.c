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

#include "flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

//  Flash implementation on RAM

const size_t __flash_size =
    KALUMA_FLASH_SECTOR_SIZE * KALUMA_FLASH_SECTOR_COUNT;
uint8_t __flash_buffer[KALUMA_FLASH_SECTOR_SIZE * KALUMA_FLASH_SECTOR_COUNT];

const uint8_t *km_flash_addr = (const uint8_t *)(__flash_buffer);

void km_flash_init() {
  for (int i = 0; i < __flash_size; i++) {
    __flash_buffer[i] = 0xFF;
  }
}

void km_flash_cleanup() {}

int km_flash_program(uint32_t sector, uint32_t offset, uint8_t *buffer,
                     size_t size) {
  const uint32_t _base = (sector * KALUMA_FLASH_SECTOR_SIZE) + offset;
  for (int i = 0; i < size; i++) {
    __flash_buffer[_base + i] = buffer[i];
  }
  return 0;
}

int km_flash_erase(uint32_t sector, size_t count) {
  const uint32_t _base = sector * KALUMA_FLASH_SECTOR_SIZE;
  for (int i = 0; i < (count * KALUMA_FLASH_SECTOR_SIZE); i++) {
    __flash_buffer[_base + i] = 0xFF;
  }
  return 0;
}
