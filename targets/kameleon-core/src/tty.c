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
#include "system.h"
#include "usbd_cdc_if.h"

static unsigned char tty_tx_buffer[TTY_TX_RINGBUFFER_SIZE];
static unsigned char tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t tty_tx_ringbuffer;
static ringbuffer_t tty_rx_ringbuffer;

ringbuffer_t * tty_get_tx_ringbuffer() {
    return &tty_tx_ringbuffer;
}

ringbuffer_t * tty_get_rx_ringbuffer() {
    return &tty_rx_ringbuffer;
}

/* this function is called in the pendable interrupt service routine which has lowest priority
   to allow other interrupts service.
*/
void tty_transmit_data() {
   
    /* if the previous data is under transmitting, just return to avoid blocking */
    if ( CDC_Transmit_IsReady() )
    {
        uint32_t len = GetDataLenInRingBuffer(&tty_tx_ringbuffer);
        if(len)
        {
            uint8_t buf[TTY_TX_RINGBUFFER_SIZE];
            ReadRingBuffer(&tty_tx_ringbuffer, buf, len);      
            CDC_Transmit_FS(buf, len);
        }
    }
}

void tty_init() {
    InitRingBuffer(&tty_tx_ringbuffer, tty_tx_buffer, sizeof(tty_tx_buffer));
    InitRingBuffer(&tty_rx_ringbuffer, tty_rx_buffer, sizeof(tty_rx_buffer));
}

void tty_putc(char ch) {
    /* (ring)buffering the string instead of transmitting it via usb channel */
    if( GetFreeSpaceInRingBuffer(&tty_tx_ringbuffer) > 1 )
    {
        FillRingBuffer(&tty_tx_ringbuffer, (uint8_t *)&ch, 1);
    }    
}

void tty_printf(const char *fmt, ...) {
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string, fmt, ap);   
    va_end(ap);

    /* (ring)buffering the string instead of transmitting it via usb channel */
    if( GetFreeSpaceInRingBuffer(&tty_tx_ringbuffer) > strlen(string) )
    {
        FillRingBuffer(&tty_tx_ringbuffer, (uint8_t *)string, strlen(string));
    }    
    SetPendSV();
}

bool tty_has_data() {
    uint32_t n = GetDataLenInRingBuffer(&tty_rx_ringbuffer);
    if(n) {
        return 1;
    }
    else {
        return 0;
    }
}

uint32_t tty_data_size() {
    // TODO:
    return GetDataLenInRingBuffer(&tty_rx_ringbuffer);
}

/** non-blocking function
*/
uint8_t tty_getc() {
    // TODO:
    uint8_t c = 0;
    if(tty_data_size()) {
        ReadRingBuffer(&tty_rx_ringbuffer, &c, 1);        
    } 
    return c;
}

/** blocking function
*/
uint8_t tty_getch() {
    // TODO:
    uint8_t c;

    while(tty_data_size() == 0);  
    ReadRingBuffer(&tty_rx_ringbuffer, &c, 1);   
    return c;
}


void tty_getstring(char * string) {
    char *string2 = string;
    char c;
    while((c = tty_getch())!='\r') {
        if(c=='\b' || c==127) {
            if( (int)string2 < (int)string )
            {
                tty_printf("\b \b");
                string--;
            }
        }
        else {
            *string++ = c;
            tty_putc(c);
        }
    }
    *string='\0';
    tty_putc('\r');   tty_putc('\n');
}

int tty_getintnum() {
    char str[256];
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
