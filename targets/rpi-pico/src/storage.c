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

#include <stdlib.h>
#include <string.h>
#include "storage.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define STORAGE_SLOT_MAX                64
#define STORAGE_SLOT_SIZE               256
#define STORAGE_DATA_MAX                STORAGE_SLOT_SIZE - 3 // Max - 3 byte
#define SIZE_FLASH_STORAGE_AREA         (STORAGE_SLOT_SIZE * STORAGE_SLOT_MAX)
#define OFFSET_FLASH_STORAGE_AREA       0x100000 - SIZE_FLASH_STORAGE_AREA
#define ADDR_FLASH_STORAGE_AREA         (XIP_BASE + OFFSET_FLASH_STORAGE_AREA)

#define STORAGE_EMPTY       0
#define STORAGE_ACTIVE      1
#define STORAGE_INACTIVE    -1

#define STORAGE_KEY_UNUSED  0xFF
typedef enum {
  STORAGE_STATUS_REMOVED    = 0x00,
  STORAGE_STATUS_USE        = 0xF0,
  STORAGE_STATUS_EMPTY      = 0xFF,
} storage_status_t;

typedef struct
{
  uint8_t status;
  uint8_t key_length;
  uint8_t data_length;
  uint8_t data[STORAGE_DATA_MAX];
} storage_data_t;

static storage_data_t* get_storage_data(uint8_t slot) {
  return (storage_data_t *)(ADDR_FLASH_STORAGE_AREA + (slot * 256));
}

static int storage_write(uint8_t slot, storage_data_t *data) {
  int status = KM_STORAGE_OK;
  uint32_t offset = OFFSET_FLASH_STORAGE_AREA + (slot * 256);
  uint32_t saved_irq = save_and_disable_interrupts();
  flash_range_program(offset, (uint8_t *)data, FLASH_PAGE_SIZE);
  restore_interrupts(saved_irq);
  return status;
}

static int get_storage_state(int slot) {
  storage_data_t *storage_data = get_storage_data(slot);
  if ((storage_data->status == STORAGE_STATUS_USE) && (storage_data->key_length != STORAGE_KEY_UNUSED))
    return STORAGE_ACTIVE;
  else if ((storage_data->status == STORAGE_STATUS_EMPTY) && (storage_data->key_length == STORAGE_KEY_UNUSED))
    return STORAGE_EMPTY; 
  else
    return STORAGE_INACTIVE;
}

static int get_number_of_storage(int status) {
  int number = 0;
  for (int i = 0; i < STORAGE_SLOT_MAX; i++) {
    if (get_storage_state(i) == status)
      number++;
  }
  return number;
}

static int get_status_of_full_storage() {
  if (get_number_of_storage(STORAGE_INACTIVE) == 0)
    return KM_STORAGE_FULL;
  else
    return KM_STORAGE_SWEEPREQ;
}

static int get_empty_storage_slot() {
  int slot = 0;
  while (get_storage_state(slot) != STORAGE_EMPTY) {
    if (++slot >= STORAGE_SLOT_MAX)
      slot = get_status_of_full_storage();
  }
  return slot;
}

static int get_slot_from_key(const char *key) {
  int slot = KM_STORAGE_ERROR;
  int input_key_length = 0;
  while (key[input_key_length] != '\0') {
    input_key_length++;
  }
  for (int i = 0; i < STORAGE_SLOT_MAX; i++) {
    if (get_storage_state(i) == STORAGE_ACTIVE) {
      storage_data_t *storage_data = get_storage_data(i);
      if (input_key_length == storage_data->key_length) {
        int length = 0;
        while (key[length] == storage_data->data[length]) {
          length++;
        }
        if (length == storage_data->key_length) {
          slot = i;
          break;
        }
      }
    }
  }
  return slot;
}

static int get_slot_from_index (int index) {
  int slot = KM_STORAGE_ERROR;
  for (int i = 0; i < STORAGE_SLOT_MAX; i++) {
    if (get_storage_state(i) == STORAGE_ACTIVE) {
      if (index == 0) {
        slot = i;
        break;
      } else {
        index--;
      }
    }
  }
  return slot;
}

/**
 * Erase all items in the storage
 * @return Return 0 on success or -1 on failture
 */
int km_storage_clear(void) {
  uint32_t saved_irq = save_and_disable_interrupts();
  flash_range_erase(OFFSET_FLASH_STORAGE_AREA, SIZE_FLASH_STORAGE_AREA);
  restore_interrupts(saved_irq);
  return KM_STORAGE_OK;
}

/**
 * Return the number of items in the storage
 * @return The number of items, or -1 on failture
 */
int km_storage_length() {
  return get_number_of_storage(STORAGE_ACTIVE);
}

/**
 * Get value of key index
 * @param key The point to key string
 * @param buf The pointer to the buffer to store value
 * @return Returns the length of value or -1 on failure (key not found)
 */
int km_storage_get_item(const char *key, char *buf) {
  int slot = get_slot_from_key(key);
  if (slot < 0)
    return slot;
  storage_data_t *storage_data = get_storage_data(slot);
  int length = storage_data->data_length;
  for (int i = 0; i < storage_data->data_length; i++) {
    buf[i] = storage_data->data[storage_data->key_length + i];
  }
  buf[length] = '\0';
  return length;
}

/**
 * Set the value with a key string
 * @param key The point to key string
 * @param buf The pointer to the buffer to store value
 * @return Returns 0 on success or -1 on failure or -2 on sweep required or -3 on full storage or -4 on over length.
 */
int km_storage_set_item(const char *key, char *buf) {
  storage_data_t *storage_data;
  int status = KM_STORAGE_OK;
  if (get_number_of_storage(STORAGE_EMPTY) == 0) {
    return get_status_of_full_storage();
  }
  int size = 0;
  while (buf[size] != '\0') {
    size++;
  }
  int slot = get_slot_from_key(key);
  if (slot >= 0) {
    storage_data = get_storage_data(slot);
    if ((size == storage_data->data_length) &&
        (memcmp(buf, &storage_data->data[storage_data->key_length], size) == 0)) {
      return KM_STORAGE_OK; /*The same data, no need to re-write */
    } else {
      if (km_storage_remove_item(key) < 0)
        status = KM_STORAGE_ERROR;
    }
  }
  storage_data = (storage_data_t *)calloc(STORAGE_SLOT_SIZE, sizeof(uint8_t));
  if (storage_data == NULL) {
    return KM_STORAGE_ERROR;
  }
  slot = get_empty_storage_slot();
  if (slot >= 0) {
    int key_length = 0;
    storage_data->status = STORAGE_STATUS_USE;
    while (key[key_length] != '\0') {
      storage_data->data[key_length] = key[key_length];
      key_length++;
    }
    if ((key_length + size) > STORAGE_DATA_MAX) {
      status = KM_STORAGE_OVERLENGTH;
    } else {
      storage_data->key_length = key_length;
      storage_data->data_length = size;
      memcpy(&storage_data->data[storage_data->key_length], buf, size);
      status = storage_write(slot, storage_data);
    }
  } else {
    status = slot;
  }
  free(storage_data);
  return status;
}

/**
 * Remove the key and value of key index
 * @param key The point to key string
 * @return Returns 0 on success or -1 on failure.
 */
int km_storage_remove_item(const char *key) {
  int status;
  int slot = get_slot_from_key(key);
  if (slot >= 0) {
    storage_data_t *old_data = get_storage_data(slot);
    storage_data_t *storage_data = (storage_data_t *)calloc(STORAGE_SLOT_SIZE, sizeof(uint8_t));
    if (storage_data == NULL) {
      return KM_STORAGE_ERROR;
    }
    memcpy(storage_data, old_data, STORAGE_SLOT_SIZE);
    storage_data->status = STORAGE_STATUS_REMOVED;
    status = storage_write(slot, storage_data);
    free(storage_data);
  } else {
    status = slot;
  }
  return status;
}


/**
 * Get key string of a given index
 * @param index The index of the key
 * @param buf The pointer to the buffer to store key string
 * @return Returns 0 on success or -1 on failure.
 */
int km_storage_key(const int index, char *buf) {
  int slot = get_slot_from_index(index);
  if (slot < 0)
    return slot;
  storage_data_t *storage_data = get_storage_data(slot);
  memcpy(buf, storage_data->data, storage_data->key_length);
  buf[storage_data->key_length] = '\0';
  return KM_STORAGE_OK;
}