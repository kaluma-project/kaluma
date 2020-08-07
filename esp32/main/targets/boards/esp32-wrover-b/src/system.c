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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <unistd.h>

#include "system.h"
#include "tty.h"
#include "kameleon_core.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "i2c.h"
#include "spi.h"
#include "uart.h"

#include <esp_wifi.h>
#include <nvs_flash.h>

const char system_arch[] = "esp32";
const char system_platform[] = "unknown";

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char * file, uint32_t line) {
  /* User can add his own implementation to report the HAL error return state */
  while(1) {
    tty_printf("_Error_Handler : file[%s], line[%d] \r\n", file, line);
    while(1);
  }
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line) {
  _Error_Handler((char *)file, line);
}

#endif

/**
*/
void delay(uint64_t msec) {
  vTaskDelay(msec/portTICK_PERIOD_MS);
}


/**
 * micro secoded delay
*/
void micro_delay(uint32_t usec) {
  // TODO
}

void inc_tick()
{
  
}

// ms 단위의 시간 return
// jerryscript setInterval 구현을 위해 필수적.
uint64_t gettime()
{
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  return (uint64_t)tv_now.tv_sec * 1E3L + (uint64_t)(tv_now.tv_usec/1E3);
}

void settime(uint64_t time)
{
}

uint32_t micro_maxtime(void)
{
  return 0;
}

uint32_t micro_gettime(void)
{
  return 0;
}

static void nvs_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
}

static void wifi_init(void)
{
   tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

extern void wifi_start(void);

/**
 * Kameleon Hardware System Initializations
 */
void system_init() {
  gpio_init(); //Should be called before LED and Button configuration
  adc_init();
  pwm_init();
  i2c_init();
  spi_init();
  uart_init();
  nvs_init();
  wifi_init();
  wifi_start();
}

void system_cleanup() {
  adc_cleanup();
  pwm_cleanup();
  i2c_cleanup();
  spi_cleanup();
  uart_cleanup();
  gpio_cleanup();
}

uint8_t running_script_check() {
  // TODO
  return true;
}

void request_firmup() {
  // TODO
}
