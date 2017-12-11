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
#include "flash.h"

#define ADDR_FLASH_USER_AREA    ((uint32_t)0x08060000)
#define ADDR_FLASH_USER_CODE    (ADDR_FLASH_USER_AREA + 4)
#define SECTOR_FLASH_USER_AREA  FLASH_SECTOR_7
#define SIZE_FLASH_USER_AREA    (128 * 1024)

uint32_t offset;

static void flush_cache() {
  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();
}

static void flash_erase() {
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = SECTOR_FLASH_USER_AREA;
  EraseInitStruct.NbSectors = 1;
  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  { 
    /* 
      Error occurred while sector erase. 
      User can add here some code to deal with this error. 
      SectorError will contain the faulty sector and then to know the code error on this sector,
      user can call function 'HAL_FLASH_GetError()'
    */
    /*
      FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError();
    */
    Error_Handler();
  }

  /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();    
}

static uint32_t calculate_checksum(uint8_t * pbuf, uint32_t size) {
  uint32_t calcurated_checksum = 0;

  for(int k=0; k<size; k++) {
    calcurated_checksum = calcurated_checksum + pbuf[k];
  }
  return (calcurated_checksum ^ (uint32_t)-1) + 1;
}

void flash_clear() {
  flash_erase();
}

uint8_t * flash_get_data() {
  return (uint8_t *)ADDR_FLASH_USER_CODE;
}

uint32_t flash_size() {
  return SIZE_FLASH_USER_AREA;
}

uint32_t flash_get_data_size() {
  uint32_t size = 0;
  uint32_t * p = (uint32_t *)ADDR_FLASH_USER_AREA;
  
  if(*p != (uint32_t)-1) {
    size = *p;
  }
  
  return size;
}

void flash_program_begin() {
  offset = 0;
  flash_erase();
}

FLASH_STATUS flash_program(uint8_t * buf, uint32_t size, uint32_t * checksum) {
  FLASH_STATUS status = FLASH_SUCCESS;
  uint32_t address, start_address, end_address;
  uint32_t k=0;
  uint8_t * p = buf;

  start_address = ADDR_FLASH_USER_CODE + offset;
  end_address = start_address + size;
  address = start_address;

  /* Unlock the Flash to enable the flash control register access */ 
  HAL_FLASH_Unlock();
  flush_cache();
  
  /* Program the user Flash area word by byte */
  while (address < end_address) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, p[k]) == HAL_OK) {
      address = address + 1;
      offset = offset + 1;
      k = k + 1;
    }
    else { 
      /* Error occurred while writing data in Flash memory. User can add here some code to deal with this error */
      /* FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
      Error_Handler();
      status = FLASH_FAIL;
      break;
    }
  }
  
  /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) */
  HAL_FLASH_Lock();

  /* create checksum */
  if(status == FLASH_SUCCESS) {
    *checksum = calculate_checksum((uint8_t *)start_address, size);
  }
  else {
    *checksum = (uint32_t)-1;
  }

  return status;
}

void flash_program_end() {
  /* Unlock the Flash to enable the flash control register access */ 
  HAL_FLASH_Unlock();
  flush_cache();
  
  /* Program the user Flash area word by word */
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ADDR_FLASH_USER_AREA, offset);
  
  /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) */
  HAL_FLASH_Lock();
}

void flash_test()
{
  flash_program_begin();

  uint32_t checksum, sum;
  { uint8_t buf[] = {0x12, 0x34, 0x56, 0x78, 0x9A};
    FLASH_STATUS status = flash_program(buf, sizeof(buf), &checksum);
    if(status != FLASH_SUCCESS)
    {
        tty_printf("FLASH ERROR \r\n");
    }
    
    sum = 0;
    for(int k=0; k<sizeof(buf); k++)
    {
        sum += buf[k];
    }
    
    tty_printf("calcurated checksum [%#010x] \r\n", checksum);    
    if( sum + checksum )
    {
          tty_printf("checksum error \r\n");
    }
  }

  { uint8_t buf[] = {0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    FLASH_STATUS status = flash_program(buf, sizeof(buf), &checksum);
    if(status != FLASH_SUCCESS)
    {
        tty_printf("FLASH ERROR \r\n");
    }

    sum = 0;
    for(int k=0; k<sizeof(buf); k++)
    {
        sum += buf[k];
    }

    tty_printf("calcurated checksum [%#010x] \r\n", checksum);    
    if( sum + checksum )
    {
          tty_printf("checksum error \r\n");
    }
  }

  flash_program_end();


  int len = flash_get_data_size();
  tty_printf("code size [%d] \r\n", len);
  
  uint8_t * p = flash_get_data();
  for(int k=0; k<len; k++)
  {
      uint8_t c = p[k];      
      tty_printf("user data : %#04x \r\n", c);    
  }  
}