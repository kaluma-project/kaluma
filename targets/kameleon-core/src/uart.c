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
#include <stdlib.h>
#include "kameleon_core.h"
#include "uart.h"
#include "ringbuffer.h"

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static UART_HandleTypeDef * uart_handle[] = {&huart1, &huart2};
static USART_TypeDef * uart_ch[] = {USART1, USART2};

static const uint32_t uart_parity[] = { UART_PARITY_NONE, UART_PARITY_ODD, UART_PARITY_EVEN };
static const uint32_t uart_hw_control[] = { UART_HWCONTROL_NONE, UART_HWCONTROL_RTS, UART_HWCONTROL_CTS, UART_HWCONTROL_RTS_CTS };
static ringbuffer_t uart_rx_ringbuffer[UART_NUM];
static uint8_t * read_buffer[] = {NULL, NULL};


/**
 * This function called by IRQ Handler
 */
void uart_fill_ringbuffer(uint8_t port, uint8_t ch) {
  ringbuffer_write(&uart_rx_ringbuffer[port], &ch, sizeof(ch));
}

/**
 * Initialize all UART when system started
 */
void uart_init() {
}

/**
 * Cleanup all UART when system cleanup
 */
void uart_cleanup() {
  for (int k = 0; k < UART_NUM; k++) {
    if (uart_handle[k]->Instance == uart_ch[k])
      uart_close(k);
  }
}

int uart_setup(uint8_t port, uint32_t baudrate, uint8_t bits,
    uart_parity_type_t parity, uint8_t stop, uart_flow_control_t flow,
    size_t buffer_size) {
  if ((port != 0) && (port != 1))
    return UARTPORT_ERROR;
  UART_HandleTypeDef * puart = uart_handle[port];

  /* UART Configuration  */
  puart->Instance = uart_ch[port];
  puart->Init.BaudRate = baudrate;
  puart->Init.WordLength = (bits == 9 ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B);
  puart->Init.StopBits = (stop == 2 ? UART_STOPBITS_2 : UART_STOPBITS_1);
  puart->Init.Parity = uart_parity[parity];
  puart->Init.HwFlowCtl = uart_hw_control[flow];
  puart->Init.Mode = UART_MODE_TX_RX;
  puart->Init.OverSampling = UART_OVERSAMPLING_16;

  read_buffer[port] = (uint8_t *)malloc(buffer_size);
  if (read_buffer[port] == NULL) {
    return UARTPORT_ERROR;
  } else {
    ringbuffer_init(&uart_rx_ringbuffer[port], read_buffer[port], buffer_size);
  }

  HAL_StatusTypeDef hal_status = HAL_UART_Init(puart);
  if (hal_status == HAL_OK) {
    __HAL_UART_ENABLE_IT(puart, UART_IT_RXNE);
    return port;
  }
  return UARTPORT_ERROR;
}


int uart_write(uint8_t port, uint8_t *buf, size_t len) {
  if ((port != 0) && (port != 1))
    return UARTPORT_ERROR;
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[port], buf, len, (uint32_t)-1);
  if (hal_status == HAL_OK) {
    return len;
  }
  return UARTPORT_ERROR;
}


uint32_t uart_available(uint8_t port) {
  if ((port != 0) && (port != 1))
    return 0;
  return ringbuffer_length(&uart_rx_ringbuffer[port]);
}


uint8_t uart_available_at(uint8_t port, uint32_t offset) {
  if ((port != 0) && (port != 1))
    return 0;
  return ringbuffer_look_at(&uart_rx_ringbuffer[port], offset);
}


uint32_t uart_buffer_size(uint8_t port) {
  if ((port != 0) && (port != 1))
    return 0;
  uint32_t size = ringbuffer_size(&uart_rx_ringbuffer[port]);
  return size;
}


uint32_t uart_read(uint8_t port, uint8_t *buf, size_t len) {
  if ((port != 0) && (port != 1))
    return 0;
  uint32_t n = ringbuffer_length(&uart_rx_ringbuffer[port]);
  if (n > len) {
    n = len;
  }
  ringbuffer_read(&uart_rx_ringbuffer[port], buf, n);
  return n;
}


int uart_close(uint8_t port) {
  if ((port != 0) && (port != 1))
    return UARTPORT_ERROR;

  if (read_buffer[port]) {
    free(read_buffer[port]);
    read_buffer[port] = (uint8_t *)NULL;
  }

  HAL_StatusTypeDef hal_status = HAL_UART_DeInit(uart_handle[port]);
  if (hal_status == HAL_OK) {
    UART_HandleTypeDef * puart = uart_handle[port];
    __HAL_UART_DISABLE_IT(puart, UART_IT_RXNE);
    return port;
  } else {
    return UARTPORT_ERROR;
  }
}
