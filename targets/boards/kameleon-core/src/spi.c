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
#include "gpio.h"

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

static SPI_HandleTypeDef * spi_handle[] = {&hspi1, &hspi3};
static SPI_TypeDef * spi_ch[] = {SPI1, SPI3};

static const uint32_t spi_prescaler[] = {SPI_BAUDRATEPRESCALER_2, SPI_BAUDRATEPRESCALER_4,
                                         SPI_BAUDRATEPRESCALER_8, SPI_BAUDRATEPRESCALER_16,
                                         SPI_BAUDRATEPRESCALER_32, SPI_BAUDRATEPRESCALER_64,
                                         SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256,
                                        };

static const uint32_t spi_firstbit[] = { SPI_FIRSTBIT_MSB, SPI_FIRSTBIT_LSB };

static uint32_t get_prescaler_factor(uint8_t bus, uint32_t baudrate) {

  uint32_t k;
  uint32_t source_clock;
  const uint32_t pre_scaler_div[] = {2, 4, 8, 16, 32, 64, 128, 256};

  if (bus==0) {
    source_clock = 96000000;
  } else {
    source_clock = 48000000;
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
int spi_setup(uint8_t bus, spi_mode_t mode, uint32_t baudrate, spi_bitorder_t bitorder, spi_bits_t bits) {

  assert_param(bus==0 || bus==1);
  assert_param(bits==SPI_8BIT || bits==SPI_16BIT);

  SPI_HandleTypeDef * pspi = spi_handle[bus];

  pspi->Instance = spi_ch[bus];
  pspi->Init.Mode = SPI_MODE_MASTER;
  switch (mode)
  {
    case SPI_MODE_0:
      pspi->Init.CLKPolarity = SPI_POLARITY_LOW;
      pspi->Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SPI_MODE_1:
      pspi->Init.CLKPolarity = SPI_POLARITY_LOW;
      pspi->Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    case SPI_MODE_2:
      pspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
      pspi->Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SPI_MODE_3:
      pspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
      pspi->Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
  }
  pspi->Init.BaudRatePrescaler = get_prescaler_factor(bus, baudrate);
  pspi->Init.FirstBit = spi_firstbit[bitorder];
  pspi->Init.Direction = SPI_DIRECTION_2LINES;
  pspi->Init.NSS = SPI_NSS_SOFT;
  pspi->Init.TIMode = SPI_TIMODE_DISABLE;
  pspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  pspi->Init.CRCPolynomial = 10;
  pspi->Init.DataSize = ((bits==SPI_16BIT) ? SPI_DATASIZE_16BIT:SPI_DATASIZE_8BIT);

  if (HAL_SPI_Init(pspi) == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

int spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx_buf, rx_buf, (uint16_t)len, timeout);

  if (status != HAL_OK) {
    return -1;
  } else {
    return (len-hspi->RxXferCount);
  }
}

int spi_send(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, buf, (uint16_t)len, timeout);

  if (status != HAL_OK) {
    return -1;
  } else {
    return len;
  }
}

int spi_recv(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  uint8_t emptyBuf[len];
  assert_param(bus==0 || bus==1);

  SPI_HandleTypeDef * hspi = spi_handle[bus];
  //I think SPI_Receive function has a bug (sending garbage data)
  //HAL_SPI_Receive(hspi, buf, (uint16_t)len, timeout);
  //Use transmitReceive with empty Tx buffer
  for (size_t i=0; i<len; i++)
    emptyBuf[i] = 0;
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, emptyBuf, buf, (uint16_t)len, timeout);
  if (status != HAL_OK) {
    return -1;
  } else {
    return (len-hspi->RxXferCount);
  }
}


int spi_close(uint8_t bus) {
  assert_param(bus==0 || bus==1);

  HAL_StatusTypeDef hal_status = HAL_SPI_DeInit(spi_handle[bus]);
  if (hal_status == HAL_OK) {
    return bus;
  } else {
    return -1;
  }
}
