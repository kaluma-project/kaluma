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

#include "storage.h"

#include <stdint.h>
#include <string.h>

#include "err.h"
#include "flash.h"

// int get_available_slot();
// storage_slot_data_t *slot_get_data(int slot);
// int slot_set_data(slot_data_t *, key, value);
// int slot_write(slot, slot_data);
// int slot_find(char *key);

static int get_available_slot() {
  const uint8_t *STORAGE_ADDR =
      (uint8_t *)(km_flash_addr + (SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
  for (int i = 0; i < SLOT_COUNT; i++) {
    storage_slot_data_t *slot_data =
        (storage_slot_data_t *)(STORAGE_ADDR + i * SLOT_SIZE);
    if (slot_data->status == SS_EMPTY) {
      return i;
    }
  }
  return -1;  // storage full
}

static storage_slot_data_t *slot_get_data(int slot) {
  const uint8_t *STORAGE_ADDR =
      (uint8_t *)(km_flash_addr + (SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
  return (storage_slot_data_t *)(STORAGE_ADDR + (slot * SLOT_SIZE));
}

static void slot_set_data(storage_slot_data_t *slot_data, char *key,
                          char *value) {
  size_t key_length = strlen(key);
  size_t value_length = strlen(value);
  memcpy(slot_data->buffer, key, key_length);
  slot_data->buffer[key_length] = '\0';
  memcpy(slot_data->buffer + key_length + 1, value, value_length);
  slot_data->buffer[key_length + value_length + 1] = '\0';
  slot_data->status = SS_USE;
  slot_data->key_length = key_length;
  slot_data->value_length = value_length;
}

static void slot_write(int slot, storage_slot_data_t *slot_data) {
  int sector = (slot * SLOT_SIZE) / KALUMA_FLASH_SECTOR_SIZE;
  int offset = (slot * SLOT_SIZE) % KALUMA_FLASH_SECTOR_SIZE;
  km_flash_program(SECTOR_BASE + sector, offset, (uint8_t *)slot_data,
                   SLOT_SIZE);
}

static int slot_find(char *key) {
  const uint8_t *STORAGE_ADDR =
      (uint8_t *)(km_flash_addr + (SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
  for (int i = 0; i < SLOT_COUNT; i++) {
    storage_slot_data_t *slot_data =
        (storage_slot_data_t *)(STORAGE_ADDR + i * SLOT_SIZE);
    if (slot_data->status == SS_USE &&
        strncmp(slot_data->buffer, key, slot_data->key_length) == 0) {
      return i;
    }
  }
  return -1;  // not found
}

static char *slot_get_key_data(storage_slot_data_t *slot_data) {
  return (char *)(slot_data->buffer);
}

static char *slot_get_value_data(storage_slot_data_t *slot_data) {
  return (char *)(slot_data->buffer + slot_data->key_length + 1);
}

int storage_set_item(char *key, char *value) {
  if (strlen(key) + strlen(value) > SLOT_DATA_MAX) {
    return ESTGSIZE;
  }
  storage_slot_data_t temp;

  // if key found, remove it
  int slot = slot_find(key);
  if (slot >= 0) {
    storage_slot_data_t *old_slot_data = slot_get_data(slot);
    memcpy(&temp, old_slot_data, sizeof(storage_slot_data_t));
    temp.status = SS_REMOVED;
    slot_write(slot, &temp);
  }

  // create new slot
  int new_slot = get_available_slot();
  if (new_slot < 0) {
    return ESTGFULL;
  } else {
    slot_set_data(&temp, key, value);
    slot_write(new_slot, &temp);
  }
  return 0;
}

int storage_get_item_value_length(char *key) {
  int slot = slot_find(key);
  if (slot < 0) {
    return ESTGNOKEY;
  }
  storage_slot_data_t *slot_data = slot_get_data(slot);
  return slot_data->value_length;
}

int storage_get_item(char *key, char *value) {
  int slot = slot_find(key);
  if (slot < 0) {
    return ESTGNOKEY;
  }
  storage_slot_data_t *slot_data = slot_get_data(slot);
  strcpy(value, slot_get_value_data(slot_data));
  return 0;
}

int storage_remove_item(char *key) {
  int slot = slot_find(key);
  if (slot < 0) {
    return ESTGNOKEY;
  }
  storage_slot_data_t *slot_data = slot_get_data(slot);
  storage_slot_data_t temp;
  memcpy(&temp, slot_data, sizeof(storage_slot_data_t));
  temp.status = SS_REMOVED;
  slot_write(slot, &temp);
  return 0;
}

int storage_clear() {
  return km_flash_erase(KALUMA_STORAGE_SECTOR_BASE,
                        KALUMA_STORAGE_SECTOR_COUNT);
}

int storage_get_item_count() {
  const uint8_t *STORAGE_ADDR =
      (uint8_t *)(km_flash_addr + (SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
  int len = 0;
  for (int i = 0; i < SLOT_COUNT; i++) {
    storage_slot_data_t *slot =
        (storage_slot_data_t *)(STORAGE_ADDR + (i * SLOT_SIZE));
    if (slot->status == SS_USE) {
      len++;
    }
  }
  return len;
}

char *storage_get_key(int index) {
  const uint8_t *STORAGE_ADDR =
      (uint8_t *)(km_flash_addr + (SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
  int idx = 0;
  for (int i = 0; i < SLOT_COUNT; i++) {
    storage_slot_data_t *slot =
        (storage_slot_data_t *)(STORAGE_ADDR + (i * SLOT_SIZE));
    if (slot->status == SS_USE) {
      if (idx == index) {
        return slot_get_value_data(slot);
      } else {
        idx++;
      }
    }
  }
  return NULL;
}
