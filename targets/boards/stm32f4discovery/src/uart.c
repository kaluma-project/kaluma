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

#include "stm32f4discovery.h"
#include "uart.h"

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

static UART_HandleTypeDef * uart_handle[] = {&huart2, &huart3};
static USART_TypeDef * uart_ch[] = {USART2, USART3};

static const uint32_t uart_data_length[] = { UART_WORDLENGTH_8B, UART_WORDLENGTH_9B };
static const uint32_t uart_parity[] = { UART_PARITY_NONE, UART_PARITY_ODD, UART_PARITY_EVEN };
static const uint32_t uart_stop_bits[] = { UART_STOPBITS_1, UART_STOPBITS_2 };
static const uint32_t uart_hw_control[] = { UART_HWCONTROL_NONE, UART_HWCONTROL_RTS, UART_HWCONTROL_CTS, UART_HWCONTROL_RTS_CTS };

/** UART Initialization
*/
int uart_setup(uint8_t bus, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, size_t buffer_size) {
  assert_param(bus==0 || bus==1);
  UART_HandleTypeDef * puart = uart_handle[bus];

  /* UART Configuration  */  
  puart->Instance = uart_ch[bus];
  puart->Init.BaudRate = baudrate;
  puart->Init.WordLength = uart_data_length[bits];
  puart->Init.StopBits = uart_stop_bits[stop];
  puart->Init.Parity = uart_parity[parity];
  puart->Init.HwFlowCtl = uart_hw_control[flow];
  puart->Init.Mode = UART_MODE_TX_RX;
  puart->Init.OverSampling = UART_OVERSAMPLING_16;
  
  int n = uart_init_ringbuffer(bus, buffer_size);
  if (n==0) {
    return -1;
  }
  
  HAL_StatusTypeDef hal_status = HAL_UART_Init(puart);
  if (hal_status == HAL_OK) {
    __HAL_UART_ENABLE_IT(puart, UART_IT_RXNE);
    return bus;
  } else {
    return -1;
  }
}

/** 
*/
int uart_write_char(uint8_t bus, uint8_t ch) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[bus], &ch, 1, (uint32_t)-1);
  if (hal_status == HAL_OK) {
    return 1;
  } else {
    return -1;
  }  
}

/** 
*/
int uart_write(uint8_t bus, uint8_t *buf, size_t len) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[bus], buf, len, (uint32_t)-1);
  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  } 
}

/** 
*/
uint32_t uart_available(uint8_t bus) {
  assert_param(bus==0 || bus==1);
  return uart_available_ringbuffer(bus);
}

/** 
*/
uint8_t uart_read_char(uint8_t bus) {
  assert_param(bus==0 || bus==1);
  return uart_read_char_ringbuffer(bus);
}

/** 
*/
uint32_t uart_read(uint8_t bus, uint8_t *buf, size_t len) {
  assert_param(bus==0 || bus==1);
  return uart_read_ringbuffer(bus, buf, len);
}

/** 
*/
int uart_close(uint8_t bus) {
  assert_param(bus==0 || bus==1);
  
  uart_deinit_ringbuffer(bus);
  
  HAL_StatusTypeDef hal_status = HAL_UART_DeInit(uart_handle[bus]);
  if (hal_status == HAL_OK) {
    UART_HandleTypeDef * puart = uart_handle[bus];
    __HAL_UART_DISABLE_IT(puart, UART_IT_RXNE);
    return bus;
  } else {
    return -1;
  }
}

//
//
//
void uart_test()
{
  int d;
  uint8_t bus = 1;
  uint8_t buf[5];
  uint32_t sec = 0;

  uart_setup(bus, 115200, UART_DATA_8_BIT, UART_PARITY_TYPE_NONE, UART_STOP_1_BIT, UART_FLOW_NONE, 1024);

  while(0)
  {
    if (uart_available(bus)) {
      d = uart_read_char(bus);
      if (d != -1) {
        uart_write_char(bus, (uint8_t)d);
      }
    }
    delay(1000);
  }

  uint32_t len;
  while( uart_available(bus) < sizeof(buf) );
  len = uart_read(bus, buf, sizeof(buf));
  uart_write(bus, buf, len);    

  uart_close(bus);
}


