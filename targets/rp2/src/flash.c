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

#include <stdlib.h>
#include <string.h>
//#include "tty.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#define HEADER_FLASH_OFFSET 0x100000
#define HEADER_FLASH_SIZE FLASH_PAGE_SIZE  // One page
#define CODE_FLASH_OFFSET HEADER_FLASH_OFFSET + HEADER_FLASH_SIZE
#define CODE_FLASH_SIZE 0x100000 - HEADER_FLASH_SIZE

#define ADDR_FLASH_USER_CODE_SIZE XIP_BASE + HEADER_FLASH_OFFSET
#define ADDR_FLASH_USER_CODE_CHECKSUM XIP_BASE + HEADER_FLASH_OFFSET + 4
#define ADDR_FLASH_USER_CODE XIP_BASE + CODE_FLASH_OFFSET

static uint32_t __code_offset;
static uint32_t __remaining_data_size;
static uint8_t *__buff;
static uint32_t __calculate_checksum(uint8_t *pbuf, uint32_t size) {
  uint32_t calcurated_checksum = 0;

  for (int k = 0; k < size; k++) {
    calcurated_checksum = calcurated_checksum + pbuf[k];
  }
  return (calcurated_checksum ^ (uint32_t)-1) + 1;
}

void km_flash_clear() {
  uint32_t saved_irq = save_and_disable_interrupts();
  flash_range_erase(HEADER_FLASH_OFFSET, HEADER_FLASH_SIZE + CODE_FLASH_SIZE);
  restore_interrupts(saved_irq);
}

uint8_t *km_flash_get_data() { return (uint8_t *)ADDR_FLASH_USER_CODE; }

void km_flash_free_data(uint8_t *data) {
  (void)data;  // Avoiding warning
}

uint32_t km_flash_size() { return CODE_FLASH_SIZE; }

uint32_t km_flash_get_data_size() {
  uint8_t *size = (uint8_t *)ADDR_FLASH_USER_CODE_SIZE;
  if (*(uint32_t *)size == 0xFFFFFFFF) {
    return 0;
  }
  return *(uint32_t *)size;
}

void km_flash_program_begin() {
  __code_offset = 0;
  __remaining_data_size = 0;
  km_flash_clear();
  __buff = (uint8_t *)malloc(FLASH_PAGE_SIZE * sizeof(uint8_t));  // 256 byte
}

km_flash_status_t km_flash_program(uint8_t *buf, uint32_t size) {
  uint32_t page_offset = 0;
  if (__buff == NULL) {
    return KM_FLASH_FAIL;
  }
  uint32_t saved_irq = save_and_disable_interrupts();
  while (size >= FLASH_PAGE_SIZE) {
    memset(__buff, 0, FLASH_PAGE_SIZE);
    memcpy(__buff + __remaining_data_size, buf + page_offset,
           FLASH_PAGE_SIZE - __remaining_data_size);
    flash_range_program(CODE_FLASH_OFFSET + __code_offset, (uint8_t *)__buff,
                        FLASH_PAGE_SIZE);
    page_offset += FLASH_PAGE_SIZE;
    __code_offset += FLASH_PAGE_SIZE;
    size -= FLASH_PAGE_SIZE;
  }
  if (size) {
    memset(__buff, 0, FLASH_PAGE_SIZE);
    memcpy(__buff, buf + page_offset, size);
    __remaining_data_size = size;
  }
  restore_interrupts(saved_irq);
  return KM_FLASH_SUCCESS;
}

void km_flash_program_end() {
  uint32_t saved_irq = save_and_disable_interrupts();
  if (__remaining_data_size) {
    flash_range_program(CODE_FLASH_OFFSET + __code_offset, (uint8_t *)__buff,
                        FLASH_PAGE_SIZE);
    __code_offset += __remaining_data_size;
  }
  free(__buff);
  uint32_t *buff =
      (uint32_t *)calloc(HEADER_FLASH_SIZE / 4, sizeof(uint32_t));  // 256 byte
  uint32_t checksum =
      __calculate_checksum((uint8_t *)ADDR_FLASH_USER_CODE, __code_offset);
  *buff = __code_offset;
  *(buff + 1) = checksum;
  flash_range_program(HEADER_FLASH_OFFSET, (uint8_t *)buff, HEADER_FLASH_SIZE);
  free(buff);
  restore_interrupts(saved_irq);
}

uint32_t km_flash_get_checksum() {
  uint8_t *cs = (uint8_t *)ADDR_FLASH_USER_CODE_CHECKSUM;
  return *(uint32_t *)cs;
}
