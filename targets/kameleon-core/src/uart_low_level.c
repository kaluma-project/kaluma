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

#include "system.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart2;

static UART_HandleTypeDef * uart_handle[] = {&huart1, &huart2};
static USART_TypeDef * uart_ch[] = {USART1, USART2};

static const uart_status_t uart_status[] = {UART_STATUS_SUCCESS, UART_STATUS_FAIL, UART_STATUS_FAIL, UART_STATUS_TIMEOUT};
static const uint32_t uart_data_length[] = { UART_WORDLENGTH_8B, UART_WORDLENGTH_9B };
static const uint32_t uart_parity[] = { UART_PARITY_NONE, UART_PARITY_ODD, UART_PARITY_EVEN };
static const uint32_t uart_stop_bits[] = { UART_STOPBITS_1, UART_STOPBITS_2 };
static const uint32_t uart_hw_control[] = { UART_HWCONTROL_NONE, UART_HWCONTROL_RTS, UART_HWCONTROL_CTS, UART_HWCONTROL_RTS_CTS };


/** Bus Valid Check
*/
static void uart_is_bus_valid(uint8_t bus) {
  if (bus!=0 && bus!=1) {
    Error_Handler();
  }
}

/** UART Transmission
*/
uart_status_t uart_transmit(uint8_t bus, uint8_t * buf, uint32_t size, uint32_t timeout) {
  uart_is_bus_valid(bus);
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[bus], buf, (uint16_t)size, timeout);
  return uart_status[hal_status];
}

/** UART Reception
*/
uart_status_t uart_receive(uint8_t bus, uint8_t * buf, uint32_t size, uint32_t timeout) {
  uart_is_bus_valid(bus);
  HAL_StatusTypeDef hal_status = HAL_UART_Receive(uart_handle[bus], buf, (uint16_t)size, timeout);
  return uart_status[hal_status];
}

/** UART Initialization
*/
void uart_init(uint8_t bus, uint32_t baudrate, uint32_t word_length, uint32_t parity, uint32_t stop_bits, uint32_t flow_control) {
  uart_is_bus_valid(bus);

  UART_HandleTypeDef * puart = uart_handle[bus];

  /* UART Configuration  */  
  puart->Instance = uart_ch[bus];
  puart->Init.BaudRate = baudrate;
  puart->Init.WordLength = uart_data_length[word_length];
  puart->Init.StopBits = uart_stop_bits[stop_bits];
  puart->Init.Parity = uart_parity[parity];
  puart->Init.HwFlowCtl = uart_hw_control[flow_control];

  puart->Init.Mode = UART_MODE_TX_RX;
  puart->Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(puart) != HAL_OK) {
    Error_Handler();
  }
}

/** 
*/
void uart_deinit(uint8_t bus) {
  uart_is_bus_valid(bus);
  HAL_UART_DeInit(uart_handle[bus]);
}
