/* Copyright (c) 2017 Kalamu
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

#include "stm32f4xx.h"
#include "kameleon_core.h"
#include "storage.h"

#define SIZE_FLASH_STORAGE_AREA         (16 * 1024)
#define ADDR_FLASH_STORAGE_AREA         (FLASH_BASE_ADDR + 0x8000)
#define SECTOR_FLASH_STORAGE_AREA       FLASH_SECTOR_2

#define STORAGE_SLOT_MAX   64
#define STORAGE_DATA_MAX    253

#define STORAGE_OK          0
#define STORAGE_ERROR       -1
#define STORAGE_SWEEPREQ    -2
#define STORAGE_FULL        -3
#define STORAGE_OVERLENGTH  -4
#define STORAGE_FATAL       -10 /* internal use */

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

/**
*/
static void flush_cache() {
  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();
}

static storage_data_t* get_storage_data(uint8_t slot) {
  return (storage_data_t *)(ADDR_FLASH_STORAGE_AREA + (slot * 256));
}

static int flash_byte_write(uint32_t addr, uint8_t data) {
  int status = STORAGE_OK;
  /* Program byte on the user Flash area */
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, data) == HAL_OK) {
  } else {
    /* Error occurred while writing data in Flash memory. User can add here some code to deal with this error */
    /* FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
    _Error_Handler(__FILE__, __LINE__);
    status = STORAGE_ERROR;
  }
  return status;
}

static int storage_write(uint8_t slot, storage_data_t data) {
  int status = STORAGE_OK;
  uint32_t address;
  address = ADDR_FLASH_STORAGE_AREA + (slot * 256);
  /* Unlock the Flash to enable the flash control register access */
  HAL_FLASH_Unlock();
  flush_cache();
  if (data.status == STORAGE_STATUS_USE) {
    if (flash_byte_write(address, data.status) == STORAGE_ERROR)
      status = STORAGE_ERROR;
    if ((status == STORAGE_OK) && (flash_byte_write(address + 1, data.key_length) != STORAGE_OK))
      status = STORAGE_FATAL;
    if ((status == STORAGE_OK) && (flash_byte_write(address + 2, data.data_length) != STORAGE_OK))
      status = STORAGE_FATAL;
    for (int i = 0; i < (data.key_length); i++) {
      if ((status == STORAGE_OK) && (flash_byte_write(address + 3 + i, data.data[i]) != STORAGE_OK))
        status = STORAGE_FATAL;
    }
    for (int i = 0; i < (data.data_length); i++) {
      if ((status == STORAGE_OK) && (flash_byte_write(address + 3 + data.key_length + i, data.data[data.key_length + i]) != STORAGE_OK))
        status = STORAGE_FATAL;
    }
    if (status == STORAGE_FATAL) {
      flash_byte_write(address, STORAGE_STATUS_REMOVED);
      status = STORAGE_ERROR;
    }
  } else {
    status = STORAGE_ERROR;
  }

  /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) */
  HAL_FLASH_Lock();
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
    return STORAGE_FULL;
  else
    return STORAGE_SWEEPREQ;
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
  int slot = STORAGE_ERROR;
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
  int slot = STORAGE_ERROR;
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

static int storage_erase(void) {
  int status = STORAGE_OK;
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = SECTOR_FLASH_STORAGE_AREA;
  EraseInitStruct.NbSectors = 1;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
    /*
      Error occurred while sector erase.
      User can add here some code to deal with this error.
      SectorError will contain the faulty sector and then to know the code error on this sector,
      user can call function 'HAL_FLASH_GetError()'
    */
    /*
      FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError();
    */
    _Error_Handler(__FILE__, __LINE__);
    status = STORAGE_ERROR; //Failure code.
  }

  /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
  return status;
}

/**
 * Erase all items in the storage
 * @return Return 0 on success or -1 on failture
 */
int storage_clear(void) {
  return storage_erase();
}

/**
 * Return the number of items in the storage
 * @return The number of items, or -1 on failture
 */
int storage_length() {
  return get_number_of_storage(STORAGE_ACTIVE);
}

/**
 * Get value of key index
 * @param key The point to key string
 * @param buf The pointer to the buffer to store value
 * @return Returns the length of value or -1 on failure (key not found)
 */
int storage_get_item(const char *key, char *buf) {
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
int storage_set_item(const char *key, char *buf) {
  int status = STORAGE_OK;
  if (get_number_of_storage(STORAGE_EMPTY) == 0) {
    return get_status_of_full_storage();
  }
  int size = 0;
  while (buf[size] != '\0') {
    size++;
  }
  int slot = get_slot_from_key(key);
  if (slot >= 0) {
    storage_data_t *storage_data = get_storage_data(slot);
    if (size == storage_data->data_length) {
      int i;
      for (i = 0; i < size; i++) {
        if (buf[i] != storage_data->data[storage_data->key_length + i]) {
          if (storage_remove_item(key) < 0)
            status = STORAGE_ERROR;
          break;
        }
      }
      if (i == size)
        return STORAGE_OK; /*The same data, no need to re-write */
    } else {
      if (storage_remove_item(key) < 0)
        status = STORAGE_ERROR;
    }
  }
  if (status == STORAGE_OK) {
    storage_data_t storage_data;
    slot = get_empty_storage_slot();
    if (slot >= 0) {
      int key_length = 0;
      storage_data.status = STORAGE_STATUS_USE;
      while (key[key_length] != '\0') {
        storage_data.data[key_length] = key[key_length];
        key_length++;
      }
      if ((key_length + size) > STORAGE_DATA_MAX) {
        status = STORAGE_OVERLENGTH;
      } else {
        storage_data.key_length = key_length;
        storage_data.data_length = size;
        for (int i = 0; i < storage_data.data_length; i++) {
          storage_data.data[storage_data.key_length + i] = buf[i];
        }
        status = storage_write(slot, storage_data);
      }
    } else {
      status = slot;
    }
  }
  return status;
}

/**
 * Remove the key and value of key index
 * @param key The point to key string
 * @return Returns 0 on success or -1 on failure.
 */
int storage_remove_item(const char *key) {
  int status;
  int slot = get_slot_from_key(key);
  if (slot >= 0) {
    uint32_t address = ADDR_FLASH_STORAGE_AREA + (slot * 256);
    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();
    flush_cache();
    status = flash_byte_write(address, STORAGE_STATUS_REMOVED);
    /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
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
int storage_key(const int index, char *buf) {
  int slot = get_slot_from_index(index);
  if (slot < 0)
    return slot;
  storage_data_t *storage_data = get_storage_data(slot);
  for (int i = 0; i < storage_data->key_length; i++) {
    buf[i] = storage_data->data[i];
  }
  buf[storage_data->key_length] = '\0';
  return STORAGE_OK;
}