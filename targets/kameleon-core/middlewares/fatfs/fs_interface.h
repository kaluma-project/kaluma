#ifndef __FS_INTERFACE_H
#define __FS_INTERFACE_H

#include <stdint.h>

#include "ffconf.h"
#include "ff.h"
#include "diskio.h"

typedef struct
{
    unsigned short year;
    unsigned char month;
    unsigned char date;
    unsigned char week_day;
    unsigned char hours;
    unsigned char minutes;
    unsigned char seconds;  
}RTC_TIME;

RTC_TIME rtc_read_time(void);
uint8_t is_disk_format_needed(void);
void disk_format_done(void);
void create_fat_volume(void);

#endif
