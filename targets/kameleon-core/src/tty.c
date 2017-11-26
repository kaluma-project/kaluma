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
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tty.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

#define BUFFER_LEN 1024

typedef struct {
	uint8_t	buf[BUFFER_LEN];
	uint32_t	r_ptr;
	uint32_t	w_ptr;
} ringbuffer_t;

static ringbuffer_t tty_ringbuffer;

/* USB Device Core handle declaration */
USBD_HandleTypeDef hUsbDeviceFS;

void tty_fill_ringbuffer(uint8_t * pbuf, uint32_t len)
{
  uint32_t k;
  ringbuffer_t * pRingBuffer = &tty_ringbuffer;

	for(k=0; k<len; k++) {
		pRingBuffer->buf[pRingBuffer->w_ptr] = pbuf[k];
		pRingBuffer->w_ptr = pRingBuffer->w_ptr + 1;
		pRingBuffer->w_ptr = pRingBuffer->w_ptr % BUFFER_LEN;
	}
}

static void tty_read_ringbuffer(uint8_t *pbuf, uint32_t len)
{
	uint32_t k;
  ringbuffer_t * pRingBuffer = &tty_ringbuffer;

	for(k=0; k<len; k++)	{
		pbuf[k] = pRingBuffer->buf[pRingBuffer->r_ptr];
		pRingBuffer->r_ptr = pRingBuffer->r_ptr + 1;
		pRingBuffer->r_ptr = pRingBuffer->r_ptr % BUFFER_LEN;
	}
}

static uint32_t tty_get_datalength_ringbuffer()
{
  uint32_t len;
  ringbuffer_t * pRingBuffer = &tty_ringbuffer;

	if(pRingBuffer->r_ptr <= pRingBuffer->w_ptr) {
		len = pRingBuffer->w_ptr - pRingBuffer->r_ptr;
	}
	else {
		len = pRingBuffer->w_ptr - pRingBuffer->r_ptr + BUFFER_LEN;
	}

	return len;
}

static void tty_init_ringbuffer()
{
  ringbuffer_t * pRingBuffer = &tty_ringbuffer;

  pRingBuffer->r_ptr = 0;
	pRingBuffer->w_ptr = 0;
}

void tty_init() {
  // TODO:
  tty_init_ringbuffer();

  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}

void tty_putc(char ch) {
  // TODO:
  while(1)
  {
      uint8_t result = CDC_Transmit_FS((uint8_t *)&ch, 1);
      if(result == USBD_OK) {
          break;
      }
  }
}

void tty_printf(const char *fmt, ...) {
  // TODO:
  va_list ap;
  char string[256];

  va_start(ap,fmt);
  vsprintf(string, fmt, ap);

  while(1)
  {
      uint8_t result = CDC_Transmit_FS((uint8_t *)string, strlen(string));
      if(result == USBD_OK) {
          break;
      }
  }
  va_end(ap);  
}

bool tty_has_data() {
  // TODO:
  if(tty_get_datalength_ringbuffer()) {
    return 1;
  }
  else {
    return 0;
  }
}

uint32_t tty_data_size() {
  // TODO:
  return tty_get_datalength_ringbuffer();
}

/** non-blocking function
*/
uint8_t tty_getc() {
  // TODO:
  uint8_t c = 0;

  if(tty_data_size()) {
     tty_read_ringbuffer(&c, sizeof(c));
  } 
  return c;
}

/** blocking function
*/
uint8_t tty_getch() {
  // TODO:
  uint8_t c;

  while(tty_data_size() == 0);  
  tty_read_ringbuffer(&c, sizeof(c));
  return c;
}


void tty_getstring(char * string)
{
    char *string2 = string;
    char c;
    while((c = tty_getch())!='\r')
    {
        if(c=='\b')
        {
            if( (int)string2 < (int)string )
            {
                tty_printf("\b \b");
                string--;
            }
        }
        else
        {
            *string++ = c;
            tty_putc(c);
        }
    }
    *string='\0';
    tty_putc('\r');   tty_putc('\n');
}

int tty_getintnum()
{
    char str[30];
    char *string = str;
    int base     = 10;
    int minus    = 0;
    int result   = 0;
    int lastIndex;
    int i;

    tty_getstring(string);

    if(string[0]=='-')
    {
        minus = 1;
        string++;
    }

    if(string[0]=='0' && (string[1]=='x' || string[1]=='X'))
    {
        base    = 16;
        string += 2;
    }

    lastIndex = strlen(string) - 1;

    if(lastIndex<0)
        return -1;

    if(string[lastIndex]=='h' || string[lastIndex]=='H' )
    {
        base = 16;
        string[lastIndex] = 0;
        lastIndex--;
    }

    if(base==10)
    {
        result = atoi(string);
        result = minus ? (-1*result):result;
    }
    else
    {
        for(i=0;i<=lastIndex;i++)
        {
            if(isalpha(string[i]))
            {
                if(isupper(string[i]))
                    result = (result<<4) + string[i] - 'A' + 10;
                else
                    result = (result<<4) + string[i] - 'a' + 10;
            }
            else
                result = (result<<4) + string[i] - '0';
        }
        result = minus ? (-1*result):result;
    }
    return result;
}
