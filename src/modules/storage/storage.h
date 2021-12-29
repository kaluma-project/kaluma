/* Copyright (c) 2017-2020 Kaluma
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

#include <stdint.h>

#include "board.h"
#include "flash.h"

#define SECTOR_BASE KALUMA_STORAGE_SECTOR_BASE
#define SECTOR_COUNT KALUMA_STORAGE_SECTOR_COUNT
#define SLOT_SIZE KALUMA_FLASH_PAGE_SIZE
#define SLOT_DATA_MAX (SLOT_SIZE - 3)
#define SLOT_COUNT ((SECTOR_COUNT * KALUMA_FLASH_SECTOR_SIZE) / SLOT_SIZE)

typedef enum {
  SS_REMOVED = 0x00,
  SS_USE = 0xF0,
  SS_EMPTY = 0xFF,
} storage_slot_status_t;

typedef struct {
  uint8_t status;
  uint8_t key_length;
  uint8_t value_length;
  char buffer[SLOT_DATA_MAX];
} storage_slot_data_t;

int storage_set_item(char *key, char *value);
int storage_get_value_length(char *key);
int storage_get_value(char *key, char *value);
int storage_get_key_length(int index);
int storage_get_key(int index, char *key);
int storage_remove_item(char *key);
int storage_clear();
int storage_get_item_count();
