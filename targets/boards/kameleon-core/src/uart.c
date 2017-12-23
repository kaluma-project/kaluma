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

#include "stm32f4xx.h"
#include "uart.h"

static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart2;

static UART_HandleTypeDef * uart_handle[] = {&huart1, &huart2};
static USART_TypeDef * uart_ch[] = {USART1, USART2};

static const uint32_t uart_data_length[] = { UART_WORDLENGTH_8B, UART_WORDLENGTH_9B };
static const uint32_t uart_parity[] = { UART_PARITY_NONE, UART_PARITY_ODD, UART_PARITY_EVEN };
static const uint32_t uart_stop_bits[] = { UART_STOPBITS_1, UART_STOPBITS_2 };
static const uint32_t uart_hw_control[] = { UART_HWCONTROL_NONE, UART_HWCONTROL_RTS, UART_HWCONTROL_CTS, UART_HWCONTROL_RTS_CTS };

/**
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
  GPIO_InitTypeDef GPIO_InitStruct;
  if (huart->Instance==USART1) {
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  } else if (huart->Instance==USART2) {
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

/**
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
  if (huart->Instance==USART1) {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
  } else if (huart->Instance==USART2) {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);

    /* USART2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}

/** UART Initialization
*/
int uart_open(uint8_t bus, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow) {
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

  HAL_StatusTypeDef hal_status = HAL_UART_Init(puart);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

/** 
*/
int uart_write_char(uint8_t bus, uint8_t ch, uint32_t timeout) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[bus], &ch, 1, timeout);
  if (hal_status == HAL_OK) {
    return 1;
  } else {
    return -1;
  }  
}

/** 
*/
int uart_write(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart_handle[bus], buf, len, timeout);
  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  } 
}

/** 
*/
int uart_read_char(uint8_t bus, uint32_t timeout) {
  assert_param(bus==0 || bus==1);
  uint8_t ch;
  HAL_StatusTypeDef hal_status = HAL_UART_Receive(uart_handle[bus], &ch, 1, timeout);
  if (hal_status == HAL_OK) {
    return (ch & 0x0ff);
  } else {
    return -1;
  } 
}

/** 
*/
int uart_read(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);
  uint8_t ch;
  HAL_StatusTypeDef hal_status = HAL_UART_Receive(uart_handle[bus], buf, len, timeout);
  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

/** 
*/
int uart_close(uint8_t bus) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_UART_DeInit(uart_handle[bus]);
  if (hal_status == HAL_OK) {
    return 0;
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
  uint8_t bus = 0;
  uint8_t buf[5];
  uint32_t timeout = 1000;

  uart_open(bus, 115200, UART_DATA_8_BIT, UART_PARITY_TYPE_NONE, UART_STOP_1_BIT, UART_FLOW_NONE);

  while(1)
  {
    d = uart_read_char(bus, timeout);
    if (d != -1) {
      uart_write_char(bus, (uint8_t)d, timeout);
    }
    else {
      tty_printf("timeout-1 \r\n");
    }

    d = uart_read(bus, buf, 1, timeout);
    if(d != -1 ) {
      uart_write(bus, buf, 1, timeout);
    }
    else {
      tty_printf("timeout-2 \r\n");
    }
  }
  uart_close(bus);
}