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
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "tty.h"
#include "system.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"


static const char* TAG = "kameleon";

void tty_init()
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_LF);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
       .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE+1,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .use_ref_tick = true
    };
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

	// 
	uart_wait_tx_done(CONFIG_ESP_CONSOLE_UART_NUM, 1000);
	uart_flush(CONFIG_ESP_CONSOLE_UART_NUM);
	printf("\r\n");
}

uint32_t tty_available()
{
  size_t len;
  if ( uart_get_buffered_data_len(CONFIG_ESP_CONSOLE_UART_NUM, &len) == ESP_OK )
    {
      return len;
    }
  return 0;
}

uint32_t tty_read(uint8_t *buf, size_t len)
{
  return read(CONFIG_ESP_CONSOLE_UART_NUM, (void*)buf, len);
}


uint32_t tty_read_sync(uint8_t *buf, size_t len, uint32_t _timeout)
{
  /*
  int state;
  fd_set readfds;
  struct timeval timeout;
  
  FD_ZERO(&readfds);
  FD_SET(0, &readfds);

  timeout.tv_sec = _timeout;
  timeout.tv_usec = 0;
  
  state = select(1, &readfds, NULL, NULL, &timeout);
  if ( state <= 0 ) return 0;

  return tty_read(buf, len);
  */
  return 0;
}


uint8_t tty_getc()
{
  return getc(stdin);
}

void tty_putc(char ch)
{
  putc(ch, stdout);
}


void tty_printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}
