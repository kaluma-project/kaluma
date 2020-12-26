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
#include <nvs_flash.h>
#include <nvs.h>
#include "esp32_devkitc.h"
#include "storage.h"


#define STORAGE_NAMESPACE ("storage")
#define STORAGE_DATA_MAX    253

#define STORAGE_OK          0
#define STORAGE_ERROR       -1
#define STORAGE_SWEEPREQ    -2
#define STORAGE_FULL        -3
#define STORAGE_OVERLENGTH  -4

int storage_clear()
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &h);
  if (err != ESP_OK) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }
  err = nvs_erase_all(h);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_erase_all\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  err = nvs_commit(h);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_commit\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  nvs_close(h);
  return STORAGE_OK;
}

int storage_length()
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &h);
  if (err != ESP_OK) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  size_t used;
  if (nvs_get_used_entry_count(h, &used) == ESP_OK)
  {
    used /= 2;
  } else {
    used = -1;
  }

  nvs_close(h);
  return used;
}

int storage_get_item(const char *key, char *buf)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &h);
  if (err != ESP_OK) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  size_t size = 256;
  err = nvs_get_str(h, key, buf, &size);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_get_str\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  nvs_close(h);
  return size;
}

int storage_set_item(const char *key, char *buf)
{
  if (strlen(buf) + strlen(key) > STORAGE_DATA_MAX)
  {
    return STORAGE_OVERLENGTH;
  }
  nvs_handle_t h;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &h);
  if (err != ESP_OK) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  err = nvs_set_str(h, key, buf);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_set_str\n", esp_err_to_name(err));
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE)
    {
      return STORAGE_FULL;
    }
    return STORAGE_ERROR;
  }

  err = nvs_commit(h);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_commit\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  nvs_close(h);
  return STORAGE_OK;
}

int storage_remove_item(const char *key)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &h);
  if (err != ESP_OK) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }
  err = nvs_erase_key(h, key);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_erase_key\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  err = nvs_commit(h);
  if (err != ESP_OK) {
    printf("ERROR (%s) nvs_commit\n", esp_err_to_name(err));
    nvs_close(h);
    return STORAGE_ERROR;
  }

  nvs_close(h);
  return STORAGE_OK;
}

int storage_key(const int index, char *buf)
{
  nvs_iterator_t it = nvs_entry_find("nvs", STORAGE_NAMESPACE, NVS_TYPE_ANY);
  for ( int i = 0 ; it != NULL && i < index ; i++ ) it= nvs_entry_next(it);

  if ( it == NULL ) {
    nvs_release_iterator(it);
    return STORAGE_ERROR;
  }

  nvs_entry_info_t info;
  nvs_entry_info(it, &info);
  strcpy(buf, info.key);

  nvs_release_iterator(it);
  return STORAGE_OK;
}

