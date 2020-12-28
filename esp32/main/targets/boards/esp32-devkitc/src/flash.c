/* Copyright (c) 2017 Kameleon
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

#include <string.h>
#include <stdlib.h>
#include "esp32_devkitc.h"
#include <esp_spiffs.h>
#include "flash.h"

esp_vfs_spiffs_conf_t flash_conf = {
  .base_path = "/spiffs",
  .partition_label = NULL,
  .max_files = 10, // What's the max?
  .format_if_mount_failed = true
};

const char* const flash_test_script =
"print(\"in flash script ok\\n\");"
;

void flash_clear()
{
#if 0
  esp_err_t err = esp_spiffs_format(flash_conf.partition_label);
  if (err != ESP_OK) {
    printf("ERROR (%s) Failed to format SPIFFS \n", esp_err_to_name(err));
  }
#else
  remove("/spiffs/info.txt");
  remove("/spiffs/index.js");
  FILE* f = fopen("/spiffs/info.txt", "w+");
  fprintf(f, "%d", 0);
  fclose(f);
  f = fopen("/spiffs/index.js", "w+");
  fclose(f);
#endif
}

uint32_t flash_size()
{
  size_t size = 0;
  size_t used = 0;
  esp_err_t err = esp_spiffs_info(flash_conf.partition_label, &size, &used);
  if (err != ESP_OK) {
    printf("ERROR (%s) Failed to get SPIFFS partition information\n", esp_err_to_name(err));
  }
  return size;
}

uint8_t * flash_get_data()
{
  FILE* f = fopen("/spiffs/info.txt", "r");
  uint32_t size = 0;
  fscanf(f, "%d", &size);
  fclose(f);
  f = fopen("/spiffs/index.js", "r");
  if (f == NULL) {
    printf("ERROR Failed to create index.js\n");
    return NULL;
  }
  uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * size);
  for (int i = 0; i < size; i++)
  { 
    *(data + i)= fgetc(f);
  }
  fclose(f);
  return data;
}

void flash_free_data(uint8_t *data) {
  free(data);
}

uint32_t flash_get_data_size()
{
  size_t size = 0;
  FILE* f = fopen("/spiffs/info.txt", "r");
  fscanf(f, "%d", &size);
  fclose(f);
  return size;
}

void flash_program_begin()
{
  flash_clear(); // format before program the file
}

flash_status_t flash_program(uint8_t* buf, uint32_t size)
{
  FILE* f = fopen("/spiffs/index.js", "w+");
  if (f == NULL) {
    printf("ERROR Failed to create index.js\n");
    return FLASH_FAIL;
  }
  FILE* f_info = fopen("/spiffs/info.txt", "w+");
  if (f == NULL) {
    printf("ERROR Failed to create info.txt\n");
    return FLASH_FAIL;
  }
  fprintf(f_info, "%d", size);
  fclose(f_info);
  for (int i = 0; i < size; i++)
  {
    fputc(buf[i], f);
  }
  fclose(f);
  return FLASH_SUCCESS;
}

flash_status_t flash_program_byte(uint8_t val)
{
  return FLASH_FAIL;
}

int flash_init(void)
{
  esp_err_t err = esp_vfs_spiffs_register(&flash_conf);
  if (err != ESP_OK) {
      printf("ERROR (%s) esp_vfs_spiffs_register\n", esp_err_to_name(err));
      return -1;
  }
  // Create file if there's no files.
  FILE* f = fopen("/spiffs/info.txt", "r");
  if (f == NULL) {
    FILE* f = fopen("/spiffs/info.txt", "w+");
    fprintf(f, "%d", 0);
  }
  f = fopen("/spiffs/index.js", "w+");
  if (f == NULL) {
    printf("ERROR Failed to create index.js\n");
    return FLASH_FAIL;
  }
  //temp
  flash_program(flash_test_script, strlen(flash_test_script));
  //temp
  fclose(f);
  return 0;
} 

void flash_program_end()
{
}

uint32_t flash_get_checksum()
{
  return 0;
}

