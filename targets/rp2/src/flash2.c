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

#include "board.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

int km_flash2_program(uint32_t sector, uint32_t offset, uint8_t *buffer,
                      size_t size) {
  const uint32_t _base =
      KALUMA_FLASH_OFFSET + (sector * KALUMA_FLASH_SECTOR_SIZE) + offset;

  // base should be multiple of KALUMA_FLASH_PAGE_SIZE
  if (_base % KALUMA_FLASH_PAGE_SIZE > 0) {
    return -1;
  }

  // size should be multiple of KALUMA_FLASH_PAGE_SIZE
  if (size % KALUMA_FLASH_PAGE_SIZE > 0) {
    return -1;
  }

  uint32_t saved_irq = save_and_disable_interrupts();
  flash_range_program(_base, buffer, size);
  restore_interrupts(saved_irq);
  return 0;
}

int km_flash2_erase(uint32_t sector, size_t count) {
  const uint32_t _base =
      KALUMA_FLASH_OFFSET + (sector * KALUMA_FLASH_SECTOR_SIZE);
  const uint32_t _size = count * KALUMA_FLASH_SECTOR_SIZE;

  // _base should be multiple of KALUMA_FLASH_PAGE_SIZE
  if (_base % KALUMA_FLASH_SECTOR_SIZE > 0) {
    return -1;
  }

  // _size should be multiple of KALUMA_FLASH_PAGE_SIZE
  if (_size % KALUMA_FLASH_SECTOR_SIZE > 0) {
    return -1;
  }

  uint32_t saved_irq = save_and_disable_interrupts();
  flash_range_erase(_base, _size);
  restore_interrupts(saved_irq);
  return 0;
}

uint8_t *km_flash2_addr() { return ((uint8_t *)KALUMA_FLASH_ADDR); }