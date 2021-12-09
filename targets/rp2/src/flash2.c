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

#include <stdlib.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

// #define KALUMA_BINARY_MAX_SIZE 0x100000
// #define KALUMA_FLASH_OFFSET 0x100000
#define KALUMA_FLASH_OFFSET 0x180000
#define KALUMA_FLASH_BASE XIP_BASE + KALUMA_FLASH_OFFSET;
#define KALUMA_FLASH_SECTOR_SIZE 4096
#define KALUMA_FLASH_SECTOR_COUNT 64
#define KALUMA_FLASH_PAGE_SIZE 256

const uint8_t *flash_target = (const uint8_t *)(KALUMA_FLASH_BASE);

int km_flash2_program(uint32_t sector, uint32_t offset, uint8_t *buffer,
                      size_t size) {
  const uint32_t _base =
      KALUMA_FLASH_BASE + (sector * KALUMA_FLASH_SECTOR_SIZE) + offset;
  if (_base % KALUMA_FLASH_PAGE_SIZE > 0) {
    // base should be multiple of KALUMA_FLASH_PAGE_SIZE
    return -1;
  }
  if (size % KALUMA_FLASH_PAGE_SIZE > 0) {
    // size should be multiple of KALUMA_FLASH_PAGE_SIZE
    return -1;
  }
  flash_range_program(_base, buffer, size);
  return 0;
}

int km_flash2_erase(uint32_t sector, size_t count) {
  flash_range_erase(KALUMA_FLASH_BASE + (sector * KALUMA_FLASH_SECTOR_SIZE),
                    count * KALUMA_FLASH_SECTOR_SIZE);
  return 0;
}
