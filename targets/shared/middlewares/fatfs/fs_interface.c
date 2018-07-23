#include "string.h"
#include "fs_interface.h"
#include "is25lq040b.h"
#include "tty.h"

static FATFS gFs;
static uint8_t disk_format_need;
uint8_t work[FF_MAX_SS];


/**
  */
RTC_TIME rtc_read_time()
{
    RTC_TIME time;    
    
    time.year = 2017;
    time.month = 11;
    time.date = 23;
    time.week_day = 5;
    
    time.hours = 14;
    time.minutes = 37;
    time.seconds = 11;
    
    return time;
}

/**
  */
FATFS * get_filesystem_object()
{
    return &gFs;  
}

/**
  */
uint32_t get_sector_size()
{
	return FF_MAX_SS;
}


/**
  */
void get_disk_space_size(uint32_t * total_size, uint32_t * free_size)
{
    FRESULT res;
    uint32_t fre_clust, fre_sect, tot_sect;
    FATFS * fs;

    fs = get_filesystem_object();
    
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree((const TCHAR *)"/", (DWORD *)&fre_clust, &fs);
    if (res)
    {
        tty_printf("get free space size error \r\n");
        while(1);
    }

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    *total_size = (uint32_t)tot_sect * get_sector_size();
    *free_size = (uint32_t)fre_sect * get_sector_size();
}


/**
  */
void create_fat_volume()
{
    FRESULT res;

    /* Create FAT Volume */
    res = f_mkfs("/", FM_ANY, 0, work, sizeof(work));
    if(res != FR_OK)
    {
        tty_printf("FAT creaton failed... [%d] \r\n", res);
        while(1);
    }
}

uint8_t is_disk_format_needed()
{
    return disk_format_need;
}

void disk_format_done()
{
    disk_format_need = 0;
}

//
//
//
void FS_SubTest()
{
	tty_printf("FS Testing \r\n");	
}

static void FS_GetDiskSizeInfoTest()
{
    uint32_t total_size, free_size;

    get_disk_space_size(&total_size, &free_size);
    tty_printf("Total Disk Size : %d Bytes \r\n", total_size);
    tty_printf("Free Disk Size : %d Bytes \r\n", free_size);
}

static void FS_EraseSpiFlash()
{
    Is25Lq_EraseChip();
}

static void FS_Mount()
{
    FRESULT res;

    /* Mount File System */
    res = f_mount(&gFs, (const char *)"/", 1);
    if( res == FR_OK )
    {
        tty_printf("File System is mounted. \r\n");
    }
    else
    {
        tty_printf("File System is not mounted. [%d] \r\n", res);
    }
}

static void FS_Format()
{
    disk_format_need = 1;
    FS_Mount();
}

static uint8_t __tty_getch() {
  while(tty_available() == 0);
  return tty_getc();
}

static void __tty_getstring(char *string) {
  char *string2 = string;
  char c;
  while ((c = __tty_getch()) != '\r') {
    if(c == '\b' || c == 127) {
      if ((int) string2 < (int) string) {
        tty_printf("\b \b");
        string--;
      }
    } else {
      *string++ = c;
      tty_putc(c);
    }
  }
  *string='\0';
  tty_putc('\r');
  tty_putc('\n');
}

static void FS_CreateDirectoryTest()
{
    char buf[64];
    FRESULT res;

    tty_printf("enter folder name to create : ");
    __tty_getstring(buf);
    
    res = f_mkdir(buf);
    if (res)
    {
        tty_printf("Create Directory Failed \r\n");
    }
}

static void FS_CreateFileTest()
{
    char buf[512];
    FIL fil;
    FRESULT res;
    uint32_t bw = 0;
    uint64_t start, end;

    for(int k=0; k<sizeof(buf); k++)    buf[k] = (char)gettime();

    tty_printf("enter file name to create : ");
    __tty_getstring(buf);
    
    start = gettime();

    res = f_open(&fil, buf, FA_WRITE | FA_CREATE_ALWAYS);
    if(res != FR_OK)
    {
        tty_printf("f_open error [%d] \r\n", res);
    }

    /* write 50kB data */
    for(int k=0; k<100; k++) {
        uint32_t w;
        res = f_write(&fil, buf, sizeof(buf), &w);
        bw = bw + w;
    }
    
    f_close(&fil);      

    end = gettime();

    tty_printf("%d bytes are written. \r\n", bw);
    tty_printf("%d elaspsed time. \r\n", (uint32_t)(end - start));
}

static void FS_CreateMultipleFileTest()
{
    FIL fil;
    FRESULT res;
    UINT bw, br;
    uint8_t buf[32];
    uint8_t data[1024];
    
    for(int k=0; k<100; k++)
    {
        sprintf((char *)buf, "%d.bin", k);
        res = f_open(&fil, (const TCHAR *)buf, FA_WRITE | FA_CREATE_ALWAYS);
        if(res != FR_OK)
        {
            tty_printf("f_open error [%d] \r\n", res);
            HAL_Delay(5000);
        }

        for(int m=0; m<sizeof(data); m++)  data[m] = k+m;
        res = f_write(&fil, data, sizeof(data), &bw);
        if(res != FR_OK)
        {
            tty_printf("f_write error [%d] \r\n", res);
            HAL_Delay(5000);
        }
        
        if(sizeof(data) != bw)
        {
            tty_printf("write size mismatch \r\n");
        }       
        
        f_close(&fil);      
    }


    for(int k=0; k<100; k++)
    {
        sprintf((char *)buf, "%d.bin", k);
        res = f_open(&fil, (const TCHAR *)buf, FA_READ);

        res = f_read(&fil, data, sizeof(data), &br);
        if(res != FR_OK)
        {
            tty_printf("f_read error [%d] \r\n", res);
            HAL_Delay(5000);
        }
        
        if(sizeof(data) != br)
        {
            tty_printf("read size mismatch \r\n");
        }       
        
        // check data
        for(int m=0; m<sizeof(data); m++)
        {
            if(data[m] != (uint8_t)(m+k))
            {
                tty_printf("data comparison error \r\n");
                while(1);
            }
        }
        
        f_close(&fil);   
        
        tty_printf("%s comparison is done \r\n", buf);
    }

}


FRESULT scan_files(char* path)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) 
    {
        for (;;) 
        {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)
            {                                               /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            }
            else 
            {                                       /* It is a file. */
              tty_printf("%s/%s \r\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

static void FS_EnumerateFilesInRootTest()
{
    char buf[256];
    
    strcpy(buf, "/");
    scan_files(buf);
}

const static void *gFsTestFunction[][2]=
{
    (void *)FS_EraseSpiFlash,           "Erase SPI Flash Memory",
    (void *)FS_Format,                  "Format Flash Memory",
    (void *)FS_Mount,                   "Mount File System",
    (void *)FS_GetDiskSizeInfoTest,     "Get total and free disk size Test",
    (void *)FS_CreateMultipleFileTest,  "Create multiple file and comparison Test",
    (void *)FS_CreateDirectoryTest,     "Create directory Test",
    (void *)FS_CreateFileTest,          "Create file Test",
    (void *)FS_EnumerateFilesInRootTest,   "Enumerate files in root Test",
    0,0
};
