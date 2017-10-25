#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

static UART_HandleTypeDef * huart;
//
//
//

char UartGetch()
{
    unsigned char ch;
    HAL_StatusTypeDef status = HAL_UART_Receive(huart, &ch, 1, -1);
    return ch;
}

char UartGetByte(char * ch)
{
    HAL_StatusTypeDef status = HAL_UART_Receive(huart, (unsigned char *)ch, 1, 0);
    if(status == HAL_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
    
void UartSendByte(char ch)
{
     HAL_UART_Transmit(huart, (uint8_t *)&ch, 1, -1);
}

void UartGetString(char *string)
{
    char *string2 = string;
    char c;
    while((c = UartGetch())!='\r')
    {
        if(c=='\b')
        {
            if( (int)string2 < (int)string )
            {
                UartPrintf("\b \b");
                string--;
            }
        }else{
            *string++ = c;
            UartSendByte(c);
        }
    }
    *string='\0';
    UartSendByte('\r');   
    UartSendByte('\n');
}


int UartGetIntNum()
{
    char str[30];
    char *string = str;
    int base     = 10;
    int minus    = 0;
    int result   = 0;
    int lastIndex;
    int i;

    UartGetString(string);

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

void UartPrintf(const char *fmt,...)
{
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);

    HAL_UART_Transmit(huart, (uint8_t *)string, strlen(string), -1);
    va_end(ap);
}

void UartDebugInit(UART_HandleTypeDef * pUartHandle)
{
    huart = pUartHandle;
}
