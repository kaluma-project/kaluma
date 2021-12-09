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

#include "flash2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define KALUMA_BINARY_MAX_SIZE 0x100000
// #define KALUMA_FLASH_OFFSET 0x100000
#define KALUMA_FLASH_OFFSET 0
#define KALUMA_FLASH_BASE KALUMA_FLASH_OFFSET
#define KALUMA_FLASH_SECTOR_SIZE 4096
#define KALUMA_FLASH_SECTOR_COUNT 64
#define KALUMA_FLASH_PAGE_SIZE 128

//  Flash implementation on RAM

const size_t __flash_size =
    KALUMA_FLASH_SECTOR_SIZE * KALUMA_FLASH_SECTOR_COUNT;
static uint8_t __flash_buffer[__flash_size];

const uint8_t *flash_target = (const uint8_t *)(__flash_buffer);

int km_flash2_program(uint32_t sector, uint32_t offset, uint8_t *buffer,
                      size_t size) {
  const uint32_t _base = (sector * KALUMA_FLASH_SECTOR_SIZE) + offset;
  for (int i = 0; i < size; i++) {
    __flash_buffer[_base + i] = buffer[i];
  }
  return 0;
}

int km_flash2_erase(uint32_t sector, size_t count) {
  const uint32_t _base = sector * KALUMA_FLASH_SECTOR_SIZE;
  for (int i = 0; i < (count * KALUMA_FLASH_SECTOR_SIZE); i++) {
    __flash_buffer[_base + i] = 0;
  }
  return 0;
}
