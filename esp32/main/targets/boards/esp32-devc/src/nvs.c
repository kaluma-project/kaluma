#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <nvs.h>

int nvs_clear(const char* namespace)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(namespace, NVS_READWRITE, &h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }
  err = nvs_erase_all(h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_erase_all\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  err = nvs_commit(h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_commit\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  nvs_close(h);  
  return 0;
}



int nvs_get_item(const char* namespace, const char *key, char *buf)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(namespace, NVS_READONLY, &h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  size_t size;
  err = nvs_get_str(h, key, buf, &size);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_get_str\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  nvs_close(h);
  return 0;
}

int nvs_get_item_alloc(const char* namespace, const char *key, char **pbuf)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(namespace, NVS_READONLY, &h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  size_t size;
  err = nvs_get_str(h, key, NULL, &size);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_get_str\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }
  char* buf = (char*)malloc(size);
  err = nvs_get_str(h, key, buf, &size);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_get_str\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }
  *pbuf = buf;
  nvs_close(h);
  return 0;
}

int nvs_get_item_len(const char* namespace, const char *key)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(namespace, NVS_READONLY, &h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  size_t size;
  err = nvs_get_str(h, key, NULL, &size);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_get_str\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }
  nvs_close(h);
  return size;
}

int nvs_set_item(const char* namespace, const char *key, const char *buf)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(namespace, NVS_READWRITE, &h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) opening NVS handle\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  err = nvs_set_str(h, key, buf);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_set_str\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  err = nvs_commit(h);
  if ( err != ESP_OK ) {
    printf("ERROR (%s) nvs_commit\n", esp_err_to_name(err));
    nvs_close(h);
    return -1;
  }

  nvs_close(h);  
  return 0;
}
