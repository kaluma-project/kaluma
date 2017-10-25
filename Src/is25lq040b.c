#include "main.h"
#include "stm32f4xx_hal.h"
#include "vcp.h"

//
//
//
#define IS25_RDID      0x9f    /* 1 Read Identification       0   0     1-3   */
#define IS25_READ      0x03    /* 1 Read Data Bytes           3   0     >=1   */
#define IS25_PP        0x02    /* 1 Page Program              3   0     1-256 */
#define IS25_SE        0x20    /* 1 Sector Erase              3   0     0     */
#define IS25_RDSR      0x05    /* 1 Read Status Register      0   0     >=1   */
#define IS25_WREN      0x06    /* 1 Write Enable              0   0     0     */

//
//
//
#define IS25_SR_WIP     (1 << 0)                /* Bit 0: Write in progress bit */

//
//
//
#define IS25_DUMMY     0xa5

#define EnableCS        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define DisableCS       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)

SPI_HandleTypeDef * gphspi;

//
//
//
void Is25Lq_Init(SPI_HandleTypeDef * hspi)
{
    gphspi = hspi;
}

void Is25Lq_WaitForComplete()
{
      uint8_t cmd = IS25_RDSR;

      EnableCS;
      HAL_SPI_Transmit(gphspi, &cmd, sizeof(cmd), -1);      
      while(1)
      {
          uint8_t r = 0;
          uint8_t t = IS25_DUMMY;
          
          HAL_SPI_TransmitReceive(gphspi, &t, &r, 1, -1);
          
          if((r & IS25_SR_WIP) ==  0)
          {
              break;
          }
      }      
      
      DisableCS;
}

void Is25Lq_WriteEnable()
{
      uint8_t cmd = IS25_WREN;

      EnableCS;
      HAL_SPI_Transmit(gphspi, &cmd, sizeof(cmd), -1);
      DisableCS;
}

//
// ID : 4 Bytes
//
void Is25Lq_ReadId(uint8_t * rx_buf)
{
      uint8_t tx_buf[] = {IS25_RDID, 
                          IS25_DUMMY, 
                          IS25_DUMMY,
                          IS25_DUMMY };
        
      Is25Lq_WaitForComplete();

      EnableCS;
      HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(gphspi, &tx_buf[0], &rx_buf[0], sizeof(tx_buf), -1);
      DisableCS;
}


uint32_t Is25Lq_ReadData(uint32_t addr, uint32_t nToRead, uint8_t * rx_buf)
{
      HAL_StatusTypeDef status;
      uint8_t cmd[] = {IS25_READ, 
                      (addr >> 16) & 0x0FF, 
                      (addr >> 8) & 0x0FF,
                      (addr >> 0) & 0x0FF };

      Is25Lq_WaitForComplete();
      
      EnableCS;
      status = HAL_SPI_Transmit(gphspi, &cmd[0], sizeof(cmd), -1);
      status = HAL_SPI_Receive(gphspi, &rx_buf[0], nToRead, -1);
      DisableCS;
      
      UNUSED(status);
      return 0;
}

uint32_t Is25Lq_WritePage(uint32_t addr, uint32_t nToWrite, uint8_t * buf)
{
      HAL_StatusTypeDef status;
      uint8_t cmd[] = {IS25_PP, 
                      (addr >> 16) & 0x0FF, 
                      (addr >> 8) & 0x0FF,
                      (addr >> 0) & 0x0FF };

      Is25Lq_WaitForComplete();
      Is25Lq_WriteEnable();
            
      EnableCS;
      status = HAL_SPI_Transmit(gphspi, &cmd[0], sizeof(cmd), -1);
      status = HAL_SPI_Transmit(gphspi, buf, nToWrite, -1);
      DisableCS;
      
      UNUSED(status);
      return 0;
}

uint32_t Is25Lq_ErasePage(uint32_t addr)
{
      HAL_StatusTypeDef status;
      uint8_t cmd[] = {IS25_SE, 
                      (addr >> 16) & 0x0FF, 
                      (addr >> 8) & 0x0FF,
                      (addr >> 0) & 0x0FF };

      Is25Lq_WaitForComplete();
      Is25Lq_WriteEnable();
      
      EnableCS;
      status = HAL_SPI_Transmit(gphspi, &cmd[0], sizeof(cmd), -1);
      DisableCS;
      
      UNUSED(status);
      return 0;
}


void Is25Lq_Test()
{
    uint8_t buf[256];
    uint32_t addr = 0x1000;
    
    //
    // Read ID
    //
    Is25Lq_ReadId(buf);
    VcpPrintf("ID : %02x %02x %02x %02x \r\n\n", buf[0], buf[1], buf[2], buf[3]);
    
    //
    // Erase Page
    //
    VcpPrintf("Erase Page \r\n");
    Is25Lq_ErasePage(addr);
    VcpPrintf("Erase Page Done. \r\n");
    
    //
    // Write Data
    //
    VcpPrintf("Write Page \r\n");
    for(int k=0; k<256; k++) buf[k] = k;
    Is25Lq_WritePage(addr, 256, buf);
    VcpPrintf("Write Page Done\r\n");

    //
    // Read Data
    //
    VcpPrintf("Read Page \r\n");
    for(int k=0; k<256; k++) buf[k] = 0;    
    Is25Lq_ReadData(addr, 256, buf);
    
    int n = 0;
    while(n < sizeof(buf))
    {
        VcpPrintf("%02x : ", n);
        for(int c=0; c<16; c++)
        {
            VcpPrintf("%02x ", buf[n++]);
        }
        VcpPrintf("\r\n");
    } 
    VcpPrintf("Read Page Done.\r\n");
}










