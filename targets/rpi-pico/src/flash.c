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
#include "tty.h"

void km_flash_clear() {
}

uint8_t * km_flash_get_data() {
  return NULL;
}

void km_flash_free_data(uint8_t *data) {
  (void)data; //Avoiding warning
}

uint32_t km_flash_size() {
  return 0;
}

uint32_t km_flash_get_data_size() {
  return 0;
}

void km_flash_program_begin() {
}

km_flash_status_t km_flash_program(uint8_t * buf, uint32_t size) {
  km_flash_status_t status = KM_FLASH_SUCCESS;
  return status;
}

km_flash_status_t km_flash_program_byte(uint8_t val) {
  km_flash_status_t status = KM_FLASH_SUCCESS;
  return status;
}

void km_flash_program_end() {
}

uint32_t km_flash_get_checksum() {
  return  0;
}
