#ifndef __IS25LQ040B_H__
#define __IS25LQ040B_H__

#include "stm32f4xx_hal.h"

//
//
//
#define IS25LQ040B_CAPACITY       0x13
#define IS25LQ040B_SECTOR_SHIFT   12    /* Sector size 1 << 12 = 4,096 */
#define IS25LQ040B_NSECTORS       128
#define IS25LQ040B_PAGE_SHIFT     8     /* Page size 1 << 8 = 256 */
#define IS25LQ040B_NPAGES         2048

#define IS25LQ040B_SECTOR_SIZE          (1 << IS25LQ040B_SECTOR_SHIFT)

void Is25Lq_Init(SPI_HandleTypeDef * hspi);
void Is25Lq_ReadId(uint8_t * rx_buf);
uint32_t Is25Lq_ReadData(uint32_t addr, uint32_t nToRead, uint8_t * rx_buf);
uint32_t Is25Lq_WritePage(uint32_t addr, uint32_t nToWrite, uint8_t * buf);
uint32_t Is25Lq_EraseChip(void);
uint32_t Is25Lq_EraseSector(uint32_t addr);


#endif
