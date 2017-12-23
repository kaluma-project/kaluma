/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include "fs_interface.h"
#include "stm32f4xx_hal.h"
#include "is25lq040b.h"

static uint8_t is_sector_erased(uint32_t sector)
{
#if (FF_MAX_SS == 512)  
	uint8_t buf[FF_MAX_SS];

	Is25Lq_ReadData(sector * FF_MAX_SS, FF_MAX_SS, buf);
	
	for(int k=0; k<FF_MAX_SS; k++)
	{
			if( buf[k] != (uint8_t)0xFF )
			{
					return 0;
			}
	}
	return 1;
    
#else
	return 0;
#endif    
  
}
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_OK;

	if( is_disk_format_needed() )
	{
			disk_format_done();
			create_fat_volume();
	}
	return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	        /* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	((void)(pdrv));

	if(count != 1)
	{
			tty_printf("check sector count value \r\n");
			while(1);
	}
	
	//tty_printf("disk_read : [%d], [%d] \r\n", sector, count);
	Is25Lq_ReadData(sector*FF_MIN_SS, count*FF_MIN_SS, buff);
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
uint8_t tmp_buf[IS25LQ040B_SECTOR_SIZE];

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		        /* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
      ((void)(pdrv));
      //tty_printf("disk_write : [%d], [%d] \r\n", sector, count);
      
      if(count != 1)
      {
          tty_printf("check sector count value \r\n");
          while(1);
      }
      
#if (FF_MAX_SS == 512)
/*      
      uint32_t start_sect = sector & ~(0x7 << 0);
      Is25Lq_ReadData(start_sect*FF_MAX_SS, sizeof(tmp_buf), tmp_buf);

      memcpy(tmp_buf + (sector & 0x7) * FF_MAX_SS, buff, FF_MAX_SS);
      
      // erase 4K bytes
      Is25Lq_EraseSector(start_sect*FF_MAX_SS);
      
      // write 4K bytes
      Is25Lq_WritePage(start_sect*FF_MAX_SS, sizeof(tmp_buf), tmp_buf);
*/
      
      if(is_sector_erased(sector))
      {
          Is25Lq_WritePage(sector*FF_MAX_SS, FF_MAX_SS, (uint8_t *)buff);
      }
      else
      {
          uint32_t start_sect = sector & ~(0x7 << 0);
          Is25Lq_ReadData(start_sect*FF_MAX_SS, sizeof(tmp_buf), tmp_buf);

          memcpy(tmp_buf + (sector & 0x7) * FF_MAX_SS, buff, FF_MAX_SS);
          
          // erase 4K bytes
          Is25Lq_EraseSector(start_sect*FF_MAX_SS);
          
          // write 4K bytes
          Is25Lq_WritePage(start_sect*FF_MAX_SS, sizeof(tmp_buf), tmp_buf);
      }
      
#elif (FF_MAX_SS == 4096)
      // do use this if the size of spi flash is more than 1MB
      // volume size
      
      // erase 4K bytes
      Is25Lq_EraseSector(sector*FF_MAX_SS);

      // write 4K bytes
      Is25Lq_WritePage(sector*FF_MAX_SS, FF_MAX_SS, (uint8_t *)buff);
      
#endif      
      
      return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_OK;
    //tty_printf("disk_ioctl : [%d] \r\n", cmd);
	
    switch (cmd) 
    {
    case CTRL_SYNC:
      res = RES_OK;
      break;
      
    case GET_SECTOR_COUNT:
      *(uint32_t *)buff = (1 << IS25LQ040B_CAPACITY) / FF_MAX_SS;
      tty_printf("GET_SECTOR_COUNT : [%d] \r\n", *(uint32_t *)buff);
      res = RES_OK;
      break;
      
    case GET_SECTOR_SIZE:
      /* never used command in case of (FF_MIN_SS == FF_MAX_SS) */      	
      *(uint32_t *)buff = FF_MAX_SS;
      res = RES_OK;
      break;
      
    case GET_BLOCK_SIZE:
    	/* must return erase block size */
			// *(uint32_t *)buff = 8;
      *(uint32_t *)buff = 1;
      tty_printf("GET_BLOCK_SIZE : [%d] \r\n", *(uint32_t *)buff);
      res = RES_OK;
      break;

    case CTRL_TRIM:
      { 
          uint32_t start = *(uint32_t *)buff;
          uint32_t end = *(uint32_t *)((uint32_t *)buff + 1);
          
          // not yet implemented !!!, disabled by default        
      }
      break;
       
    default:
      res = RES_PARERR;
    }
	  return res;
}

DWORD get_fattime()
{
	RTC_TIME time = rtc_read_time();
	return	((uint32_t)(time.year - 1980) << 25)
				| ((uint32_t)time.month << 21)
				| ((uint32_t)time.date << 16)
				| ((uint32_t)time.hours << 11)
				| ((uint32_t)time.minutes << 5)
				| ((uint32_t)time.seconds >> 1);	
} 
									
