#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "buffer.h"
#include "usbd_cdc_if.h"
#include "vcp.h"

static RINGBUFFER gVcpReceiveBuffer;

//
//
//
unsigned int AddVcpReceiveData(unsigned char * pBuf, unsigned int n)
{
    FillRingBuffer(&gVcpReceiveBuffer, pBuf, n);
    return GetDataLenInRingBuffer(&gVcpReceiveBuffer);
}

char VcpGetch()
{
    unsigned char ch;
    
    while(GetDataLenInRingBuffer(&gVcpReceiveBuffer)==0)
    {
    }
    
    ReadRingBuffer(&gVcpReceiveBuffer, &ch, sizeof(ch));
    return ch;
}

char VcpGetByte()
{
    char ch;
    ReadRingBuffer(&gVcpReceiveBuffer, (unsigned char *)&ch, sizeof(ch));
    return ch;
}

    
void VcpSendByte(char ch)
{
    while(1)
    {
        uint8_t result = CDC_Transmit_FS((uint8_t *)&ch, sizeof(ch));
        if(result == USBD_OK)
        {
            break;
        }
    }
}

char VcpIsCharReceived()
{
    if( GetDataLenInRingBuffer(&gVcpReceiveBuffer) )
    {
          return 1;
    }
    else
    {
          return 0;
    }
}


void VcpGetString(char *string)
{
    char *string2 = string;
    char c;
    while((c = VcpGetch())!='\r')
    {
        if(c=='\b')
        {
            if( (int)string2 < (int)string )
            {
                VcpPrintf("\b \b");
                string--;
            }
        }else{
            *string++ = c;
            VcpSendByte(c);
        }
    }
    *string='\0';
    VcpSendByte('\r');   
    VcpSendByte('\n');
}


int VcpGetIntNum()
{
    char str[30];
    char *string = str;
    int base     = 10;
    int minus    = 0;
    int result   = 0;
    int lastIndex;
    int i;

    VcpGetString(string);

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
    }else{
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

void VcpPrintf(const char *fmt,...)
{
#if 1  
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);

    while(1)
    {
        uint8_t result = CDC_Transmit_FS((uint8_t *)string, strlen(string));
        if(result == USBD_OK)
        {
            break;
        }
    }
    va_end(ap);
#else

    printf(fmt);
#endif    
}

void VcpDebugInit(UART_HandleTypeDef * pUartHandle)
{
    InitRingBuffer(&gVcpReceiveBuffer);
}
