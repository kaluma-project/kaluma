#include <stdint.h>

#include "board.h"
#include "flash2.h"

int km_program_clear() {
  return km_flash2_erase(KALUMA_PROGRAM_SECTOR_BASE,
                         KALUMA_PROGRAM_SECTOR_COUNT);
}

int km_program_begin() { return 0; }

int km_program_write(int8_t *buffer, int size) { return 0; }

int km_program_end() { return 0; }

int km_program_get_size() { return 0; }

int km_program_max_size() {
  return (KALUMA_PROGRAM_SECTOR_COUNT * KALUMA_FLASH_SECTOR_SIZE);
}

uint8_t *km_program_addr() {
  return (uint8_t *)km_flash2_addr() +
         (KALUMA_PROGRAM_SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE);
}
