#ifndef __IS25LQ040_H__
#define __IS25LQ040_H__

void Is25Lq_Init(SPI_HandleTypeDef * hspi);
uint8_t Is25Lq_ReadId(uint8_t * rx_buf);
uint32_t Is25Lq_ReadData(uint32_t addr, uint32_t nToRead, uint8_t * rx_buf);

#endif
