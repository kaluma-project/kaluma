#include "prog.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "flash2.h"

// TODO: JUST WRITE CODE STRING FROM START (NO HEADER AREA)
// TODO: DONT' NEED CHECKSUM
// TODO: USE strlen() to get length of code string

// THE STRUCTURE OF FLASH AREA FOR USER PROGRAM (JS)
// |--------|-------------------------------------------|
// | HEADER |                  CODE                     |
// |--------|-------------------------------------------|
// | 1 PAGE |                 n PAGES                   |
// |--------|-------------------------------------------|
// |                   n SECTORS                        |
// |----------------------------------------------------|

static uint8_t *page_buffer = NULL;
static uint32_t page_written = 0;
static uint32_t total_written = 0;

static int page_buffer_flush() {
  int sector = total_written / KALUMA_FLASH_SECTOR_SIZE;
  int offset = total_written % KALUMA_FLASH_SECTOR_SIZE;
  int ret = km_flash2_program(KALUMA_PROGRAM_SECTOR_BASE + sector, offset,
                              page_buffer, page_written);
  if (ret < 0) return ret;
  memset(page_buffer, 0, KALUMA_FLASH_PAGE_SIZE);
  page_written = 0;
  return 0;
}

static int page_buffer_push(uint8_t byte) {
  page_buffer[page_written] = byte;
  page_written++;
  total_written++;
  if (page_written >= KALUMA_FLASH_PAGE_SIZE) {
    page_buffer_flush();
  }
  return 0;
}

int km_prog_clear() {
  return km_flash2_erase(KALUMA_PROGRAM_SECTOR_BASE,
                         KALUMA_PROGRAM_SECTOR_COUNT);
}

int km_prog_begin() {
  page_buffer = malloc(KALUMA_FLASH_PAGE_SIZE);
  memset(page_buffer, 0, KALUMA_FLASH_PAGE_SIZE);
  page_written = 0;
  total_written = 0;
  return 0;
}

int km_prog_write(int8_t *buffer, int size) {
  for (int i = 0; i < size; i++) {
    int ret = page_buffer_push(buffer[i]);
    if (ret < 0) {
      return ret;
    }
  }
  return 0;
}

int km_prog_end() {
  if (page_written > 0) {
    page_buffer_flush();
  }
  if (page_buffer != NULL) {
    free(page_buffer);
  }
  total_written = 0;
  return 0;
}

int km_prog_get_size() {
  char *prog = (char *)km_prog_addr();
  if (prog[0] == 255) {  // flash erased (no code written)
    return 0;
  }
  return strlen(prog);
}

int km_prog_max_size() {
  return (KALUMA_PROGRAM_SECTOR_COUNT * KALUMA_FLASH_SECTOR_SIZE);
}

uint8_t *km_prog_addr() {
  return (uint8_t *)(km_flash_addr +
                     (KALUMA_PROGRAM_SECTOR_BASE * KALUMA_FLASH_SECTOR_SIZE));
}
