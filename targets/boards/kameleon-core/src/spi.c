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

#include "kameleon_core.h"
#include "spi.h"
#include "gpio_low_level.h"

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

static SPI_HandleTypeDef * spi_handle[] = {&hspi1, &hspi2};
static SPI_TypeDef * spi_ch[] = {SPI1, SPI2};

static const uint32_t spi_mode[] = {SPI_MODE_MASTER, SPI_MODE_SLAVE};
static const uint32_t spi_clock_polarity[] = {SPI_POLARITY_LOW, SPI_POLARITY_HIGH};
static const uint32_t spi_clock_phase[] = {SPI_PHASE_1EDGE, SPI_PHASE_2EDGE};
static const uint32_t spi_prescaler[] = {SPI_BAUDRATEPRESCALER_2, SPI_BAUDRATEPRESCALER_4,
                                         SPI_BAUDRATEPRESCALER_8, SPI_BAUDRATEPRESCALER_16,
                                         SPI_BAUDRATEPRESCALER_32, SPI_BAUDRATEPRESCALER_64,
                                         SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256,
                                        };
static const uint32_t spi_firstbit[] = { SPI_FIRSTBIT_MSB, SPI_FIRSTBIT_LSB };

static GPIO_TypeDef * spi_cs_port;
static uint32_t spi_cs_pin;


/**
*/
static void enable_cs() {
  HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_RESET);
}

/**
*/
static void disable_cs() {
  HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_SET);
}

/**
*/
static uint32_t get_prescaler_factor(uint8_t bus, uint32_t baudrate) {

  uint32_t k;
  uint32_t source_clock;
  const uint32_t pre_scaler_div[] = {2, 4, 8, 16, 32, 64, 128, 256};
  
  if (bus==0) {
    source_clock = 42000000;
  } else {
    source_clock = 21000000;
  }

  for (k=0; k<(sizeof(pre_scaler_div)/sizeof(uint32_t)); k++) {
    uint32_t rate = source_clock / pre_scaler_div[k];
    if (baudrate >= rate) {
      break;
    }    
  }
  
  if (k==sizeof(pre_scaler_div)/sizeof(uint32_t)) {
    k=k-1;
  }
  return spi_prescaler[k];
}

/** SPI Setup
*/
int spi_setup(uint8_t bus, spi_mode_t mode, uint8_t cs_pin, uint32_t baudrate, 
              spi_clock_polarity_t polarity, spi_clock_phase_t phase, uint8_t bits, 
              spi_bitorder_t bit_order) {
                
  assert_param(bus==0 || bus==1);
  assert_param(bits==8 || bits==16);
  
  SPI_HandleTypeDef * pspi = spi_handle[bus];
    
  pspi->Instance = spi_ch[bus];
  pspi->Init.Mode = spi_mode[mode];        
  pspi->Init.CLKPolarity = spi_clock_polarity[polarity];
  pspi->Init.CLKPhase = spi_clock_phase[phase];
  pspi->Init.BaudRatePrescaler = get_prescaler_factor(bus, baudrate);
  pspi->Init.FirstBit = spi_firstbit[bit_order];
  pspi->Init.Direction = SPI_DIRECTION_2LINES;
  pspi->Init.NSS = SPI_NSS_SOFT;    
  pspi->Init.TIMode = SPI_TIMODE_DISABLE;
  pspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  pspi->Init.CRCPolynomial = 10;
  pspi->Init.DataSize = ((bits==8) ? SPI_DATASIZE_8BIT:SPI_DATASIZE_16BIT);

  if (HAL_SPI_Init(pspi) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* configure cs pin as gpio-output
  */
  spi_cs_port = get_gpio_port(cs_pin);
  spi_cs_pin = get_gpio_pin(cs_pin);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = spi_cs_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(spi_cs_port, &GPIO_InitStruct);   
  disable_cs();
  
  return 0;
}

/**
*/
int spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  
  enable_cs();
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx_buf, rx_buf, (uint16_t)len, timeout);
  disable_cs();
  
  if (status != HAL_OK) {
    return -1;
  } else {
    return (len-hspi->RxXferCount);
  }  
}

/**
*/
int spi_send(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  
  enable_cs();
  HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, buf, (uint16_t)len, timeout);
  disable_cs();
  
  if (status != HAL_OK) {
    return -1;
  } else {
    return len;
  }  
}

/**
*/
int spi_recv(uint8_t bus, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  HAL_SPI_Receive(hspi, buf, (uint16_t)len, timeout);
  return (len - hspi->RxXferCount);
}


/**
*/
int spi_close(uint8_t bus) {
  assert_param(bus==0 || bus==1);

  HAL_StatusTypeDef hal_status = HAL_SPI_DeInit(spi_handle[bus]);
  if (hal_status == HAL_OK) {
    return bus;
  } else {
    return -1;
  }
}

//
//
//
void spi_test()
{
  uint8_t bus = 0;
  
  spi_setup(bus, SPI_SETUP_MODE_MASTER, 0, 4000000, 
            SPI_CLOCK_POLARITY_LOW, SPI_CLOCK_PHASE_1_EDGE, 8,
            SPI_BITORDER_MSB);
  
  // L3G4200D (WHO AM I ==> 0xD3)
  uint8_t tx_buf[] = {0x80|0x0F, 0xFF};
  uint8_t rx_buf[] = {0, 0};
  int n = spi_sendrecv(bus, tx_buf, rx_buf, 2, (uint32_t)-1);

  tx_buf[0] = 0x20;
  tx_buf[1] = 0x0F;
  n = spi_sendrecv(bus, tx_buf, rx_buf, 2, (uint32_t)-1);

  while(1)
  {
    uint16_t x, y, z;
    
    tx_buf[0] = 0x80 | 0x29;
    tx_buf[1] = 0x00;
    spi_sendrecv(bus, tx_buf, rx_buf, 2, (uint32_t)-1);
    x = rx_buf[1] << 8;
    
    tx_buf[0] = 0x80 | 0x28;
    tx_buf[1] = 0x00;
    spi_sendrecv(bus, tx_buf, rx_buf, 2, (uint32_t)-1);
    x = x | rx_buf[1];

    printf("X : %d \r\n", x);
    
    delay(1000);
  }
  
  
  spi_close(bus);
}



