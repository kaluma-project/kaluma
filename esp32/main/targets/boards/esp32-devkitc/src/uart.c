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
#include <stdlib.h>
#include <string.h>
#include "esp32/rom/uart.h"
#include "esp32_devkitc.h"
#include "driver/uart.h"
#include "ringbuffer.h"
#include "uart.h"
#include "esp_intr_alloc.h"

static ringbuffer_t uart_rx_ringbuffer[UART_NUM];
static uint8_t * read_buffer[] = {NULL, NULL};
static intr_handle_t handle_console[UART_NUM];
/*
 * Define UART interrupt subroutine to ackowledge interrupt
 */
static void IRAM_ATTR uart_intr_handle_1(void *arg)
{
  uint16_t rx_fifo_len;
  uint8_t ch;
  rx_fifo_len = UART1.status.rxfifo_cnt; // read number of bytes in UART buffer

  while(rx_fifo_len) {
    ch = UART1.fifo.rw_byte;
    ringbuffer_write(&uart_rx_ringbuffer[0], &ch, sizeof(ch));
    rx_fifo_len--;
  }

  // after reading bytes from buffer clear UART interrupt status
  uart_clear_intr_status(UART_NUM_1, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
}

/*
 * Define UART interrupt subroutine to ackowledge interrupt
 */
static void IRAM_ATTR uart_intr_handle_2(void *arg)
{
  uint16_t rx_fifo_len;
  uint8_t ch;
  rx_fifo_len = UART2.status.rxfifo_cnt; // read number of bytes in UART buffer
  
  while(rx_fifo_len) {
    ch = UART2.fifo.rw_byte;
    ringbuffer_write(&uart_rx_ringbuffer[1], &ch, sizeof(ch));
    rx_fifo_len--;
  }

  // after reading bytes from buffer clear UART interrupt status
  uart_clear_intr_status(UART_NUM_2, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
}

void uart_init()
{
}

void uart_cleanup()
{
  for (int i = 0; i < UART_NUM; i++) {
    uart_close(i);
  }
}

int uart_setup(uint8_t port, uint32_t baudrate, uint8_t bits,
  uart_parity_type_t parity, uint8_t stop, uart_flow_control_t flow,
  size_t buffer_size)
{
  uint8_t rx_pin, tx_pin, rts_pin, cts_pin, esp_port;
  if ((port > 1) || (bits != 8)) {
    return UARTPORT_ERROR; //Only 8 bit is allowed.
  }
  if (port == 0) {
    rx_pin = 16;
    tx_pin = 17;
    rts_pin = 4;
    cts_pin = 34;
    esp_port = 1; //ESP UART 1
  } else {
    rx_pin = 2;
    tx_pin = 0;
    rts_pin = 15;
    cts_pin = 35;
    esp_port = 2; //ESP UART 2
  }
  if (parity == UART_PARITY_TYPE_ODD) {
    parity = UART_PARITY_ODD;
  } else if (parity == UART_PARITY_TYPE_EVEN) {
    parity = UART_PARITY_EVEN;
  } else {
    parity = UART_PARITY_DISABLE;
  }
  if (stop == 2) {
    stop = UART_STOP_BITS_2;
  } else {
    stop = UART_STOP_BITS_1;
  }
  const uart_config_t uart_config = {
      .baud_rate = baudrate,
      .data_bits = UART_DATA_8_BITS,
      .parity = parity,
      .stop_bits = stop,
      .flow_ctrl = flow,
      .rx_flow_ctrl_thresh = 122,
  };
  read_buffer[port] = (uint8_t *)malloc(buffer_size);
  if (read_buffer[port] == NULL) {
    return UARTPORT_ERROR;
  } else {
    ringbuffer_init(&uart_rx_ringbuffer[port], read_buffer[port], buffer_size);
  }
  uart_param_config(esp_port, &uart_config);
  uart_set_pin(esp_port, tx_pin, rx_pin, rts_pin, cts_pin);
  uart_driver_install(esp_port, buffer_size * 2, 0, 0, NULL, 0);
  uart_isr_free(esp_port);
  if (esp_port == 1)
  {
    uart_isr_register(UART_NUM_1, uart_intr_handle_1, NULL, ESP_INTR_FLAG_IRAM, &handle_console[0]);
  } else {
    uart_isr_register(UART_NUM_2, uart_intr_handle_2, NULL, ESP_INTR_FLAG_IRAM, &handle_console[1]);
  }
    uart_enable_rx_intr(esp_port);
  return 0;
}

int uart_write(uint8_t port, uint8_t *buf, size_t len)
{
  uint8_t esp_port = port + 1; // system port 0 is esp port 1
  if (port > 1) {
    return UARTPORT_ERROR; //Only 8 bit is allowed.
  }
  return uart_write_bytes(esp_port, buf, len);
}

uint32_t uart_available(uint8_t port)
{
  if (port > 1) {
    return 0; //Only 8 bit is allowed.
  }
  return ringbuffer_length(&uart_rx_ringbuffer[port]);
}

uint8_t uart_available_at(uint8_t port, uint32_t offset)
{
  if (port > 1) {
    return 0; //Only 8 bit is allowed.
  }
  return ringbuffer_look_at(&uart_rx_ringbuffer[port], offset);
}

uint32_t uart_buffer_size(uint8_t port)
{
  if (port > 1) {
    return 0; //Only 8 bit is allowed.
  }
  return ringbuffer_size(&uart_rx_ringbuffer[port]);
}

uint32_t uart_read(uint8_t port, uint8_t *buf, size_t len)
{
  if (port > 1) {
    return UARTPORT_ERROR; //Only 8 bit is allowed.
  }
  uint32_t n = ringbuffer_length(&uart_rx_ringbuffer[port]);
  if (n > len) {
    n = len;
  }
  ringbuffer_read(&uart_rx_ringbuffer[port], buf, n);
  return n;
}

int uart_close(uint8_t port)
{
  uint8_t esp_port = port + 1;
  if (read_buffer[port]) {
    free(read_buffer[port]);
    read_buffer[port] = (uint8_t *)NULL;
  }
  uart_driver_delete(esp_port);
  return 0;
}
 
