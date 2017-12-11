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

#include "system.h"
#include "system_low_level.h"
#include "tty_low_level.h"
#include "usbd_cdc_if.h"

void tty_init() {
    tty_init_ringbuffer();
}

void tty_putc(char ch) {
    /* (ring)buffering the string instead of transmitting it via usb channel */
    if( tty_get_tx_freespace() > 1 )
    {
        tty_put_byte(ch);
    }
    SetPendSV(); 
}

void tty_printf(const char *fmt, ...) {
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string, fmt, ap);   
    va_end(ap);

    /* (ring)buffering the string instead of transmitting it via usb channel */
    if( tty_get_tx_freespace() > strlen(string) )
    {
        tty_put_bytes((uint8_t *)string, strlen(string));
    }    
    SetPendSV();
}

bool tty_has_data() {
    uint32_t n = tty_get_rx_data_length();
    if(n) {
        return 1;
    }
    else {
        return 0;
    }
}

uint32_t tty_data_size() {
    // TODO:
    return tty_get_rx_data_length();
}

/** non-blocking function
*/
uint8_t tty_getc() {
    // TODO:
    uint8_t c = 0;
    if(tty_data_size()) {
        c = tty_get_byte();       
    } 
    return c;
}

/** blocking function
*/
uint8_t tty_getch() {
    while(tty_data_size() == 0);  
    return tty_get_byte();
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
