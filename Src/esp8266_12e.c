#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart.h"


#define ESP_RX_BUFFER_LEN       2048

typedef enum
{
    AT_NULL = 0,
    AT_GMR,
    AT_CWMODE,
    AT_CWJAP_QUIRY,
    AT_CWJAP,
    AT_CWLAP,
    AT_CIFSR,
    AT_CIPSTA_CUR_QUIRY,
    AT_PING,
    AT_CIPSTART,
    AT_CIPSEND,
    AT_SLEEP,
    AT_GSLP,
    AT_ATE0,
}AT_CMD;


typedef enum
{
    RESPONSE_NO_RESPONSE,
    RESPONSE_OK,
    RESPONSE_ERROR,
    RESPONSE_FAIL,
}RESPONSE;

void Esp8266_Reset(void);
void Esp8266_SendCmdTest(AT_CMD);
//
//
//
static UART_HandleTypeDef * huart;
static DMA_HandleTypeDef * hdma_uart_rx;
static unsigned int gReadPtrInRxDMABuffer;
static unsigned char gRxBuffer[ESP_RX_BUFFER_LEN];

static const void *function[][2]=
{
	(void *)Esp8266_SendCmdTest,	"AT Command",
	(void *)Esp8266_SendCmdTest,	"AT+GMR (Version Info.) Command",
	(void *)Esp8266_SendCmdTest,    "AT+CWMODE (Set WiFi Mode) Command",
	(void *)Esp8266_SendCmdTest,    "AT+CWJAP? (Display Connected AP Info.) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CWJAP (Connect to AP) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CWLAP (List available APs) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CIFSR (Query IP Address TCP/IP Layer) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CIPSTA_CUR? (Query IP Address WiFi Layer) Command",
	(void *)Esp8266_SendCmdTest,	"AT+PING (Ping) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CIPSTART (TCP Client Connnect) Command",
	(void *)Esp8266_SendCmdTest,	"AT+CIPSEND (TCP Client Transmit) Command",
	(void *)Esp8266_SendCmdTest,	"AT+SLEEP (Sleep) Command",
	(void *)Esp8266_SendCmdTest,	"AT+GSLP (Deep Sleep) Command",
	(void *)Esp8266_Reset,  	"ESP8266 Reset",
	0,0
};


unsigned int Esp8266_GetDataLengthInBuffer()
{
    unsigned int nCount;  
    unsigned int nWritePtr = huart->RxXferSize - __HAL_DMA_GET_COUNTER(hdma_uart_rx); 

    if( gReadPtrInRxDMABuffer < nWritePtr )
    {
        nCount = nWritePtr - gReadPtrInRxDMABuffer;
    }
    else if( nWritePtr < gReadPtrInRxDMABuffer )
    {
        nCount = huart->RxXferSize - ( gReadPtrInRxDMABuffer - nWritePtr );
    }
    else
    {
        nCount = 0;
    }
    
    return nCount;
}

void Esp8266_GetBytes(unsigned char * pBuf, unsigned int len)
{
    for(int k=0; k<len; k++)
    {
        pBuf[k] = gRxBuffer[gReadPtrInRxDMABuffer];
        gReadPtrInRxDMABuffer = (gReadPtrInRxDMABuffer + 1) % huart->RxXferSize;
    }
}

unsigned int Esp8266_IsByteReceived()
{   
    unsigned int nWritePtr = huart->RxXferSize - __HAL_DMA_GET_COUNTER(hdma_uart_rx); 
  
    if( gReadPtrInRxDMABuffer != nWritePtr )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void Esp8266_Printf(const char *fmt,...)
{
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    HAL_UART_Transmit(huart, (uint8_t *)string, strlen(string), -1);
    va_end(ap);
}

void Esp8266_SendByte(char ch)
{
    HAL_UART_Transmit(huart, (uint8_t *)&ch, 1, -1);
}

RESPONSE Esp8266_WaitResponse()
{
    RESPONSE res = RESPONSE_NO_RESPONSE;
    uint32_t k = 0;
    uint8_t buf[1024];
    
    while(1)
    {
        unsigned int len = Esp8266_GetDataLengthInBuffer();
        if(len)
        {
            Esp8266_GetBytes(buf+k, len);
            k = k + len;
            
            //
            //
            //
            if( (k >= 4) &&
                (buf[k-1] == '\n') &&
                (buf[k-2] == '\r') &&
                (buf[k-3] == 'K') &&
                (buf[k-4] == 'O') )
            {
                buf[k] = 0x00;
                k = k + 1;
                break;
            }
            else if((k >= 7) &&
                    (buf[k-1] == '\n') &&
                    (buf[k-2] == '\r') &&
                    (buf[k-3] == 'R') &&
                    (buf[k-4] == 'O') &&
                    (buf[k-5] == 'R') &&
                    (buf[k-6] == 'R') &&
                    (buf[k-7] == 'E') )
            {
                buf[k] = 0x00;
                k = k + 1;
                break;
            }
        }
    }

    //
    //
    //
    for(int n=0; n<k; n++)
    {
        UartSendByte(buf[n]);
    }

    if(strstr((char *)buf, "OK\r\n") != NULL)
    {
        res = RESPONSE_OK;
    }
    else if(strstr((char *)buf, "ERROR\r\n") != NULL)
    {
        res = RESPONSE_ERROR;
    }
    else if(strstr((char *)buf, "FAIL\r\n") != NULL)
    {
        res = RESPONSE_FAIL;
    }
    else
    {
        res = RESPONSE_NO_RESPONSE;
    }
    
    return res;
}

void Esp8266_CmdHandler(AT_CMD cmd)
{
    char buf[128];
    char payload[128];
    
    if(cmd == AT_NULL)
    {
        strcpy(buf, "AT");
    }
    else if(cmd == AT_GMR)
    {
        strcpy(buf, "AT+GMR");
    }
    else if(cmd == AT_ATE0)
    {
        strcpy(buf, "AT+ATE0");
    }
    else if(cmd == AT_CWMODE)
    {
        UartPrintf("Select WiFi Mode (1)Station, (2)SoftAP (3) SoftAP+Station : ");
        
        int num = UartGetIntNum();
        if(num == 1)            strcpy(buf, "AT+CWMODE=1");
        else if(num == 2)       strcpy(buf, "AT+CWMODE=2");
        else if(num == 3)       strcpy(buf, "AT+CWMODE=3");
    }
    else if(cmd == AT_CWJAP)
    {   
        char ssid[32], passwd[32];

        UartPrintf("Enter SSID : ");
        UartGetString(ssid);

        UartPrintf("Enter PASSWORD : ");
        UartGetString(passwd);

        strcpy(buf, "AT+CWJAP=");
        strcat(buf, "\"");
        strcat(buf, ssid);
        strcat(buf, "\"");
        strcat(buf, ",");
        strcat(buf, "\"");
        strcat(buf, passwd);
        strcat(buf, "\"");
    }
    else if(cmd == AT_CWJAP_QUIRY)
    {
        strcpy(buf, "AT+CWJAP?");
    }
    else if(cmd == AT_CIPSTA_CUR_QUIRY)
    {
        strcpy(buf, "AT+CIPSTA_CUR?");
    }
    else if(cmd == AT_CWLAP)
    {
        strcpy(buf, "AT+CWLAP");
    }
    else if(cmd == AT_CIFSR)
    {
        strcpy(buf, "AT+CIFSR");
    }
    else if(cmd == AT_PING)
    {
        char ip[16];

        UartPrintf("Enter IP : ");
        UartGetString(ip);

        strcpy(buf, "AT+PING=");
        strcat(buf, "\"");
        strcat(buf, ip);
        strcat(buf, "\"");
    }
    else if(cmd == AT_CIPSTART)
    {
        char ip[16], port[8];

        UartPrintf("Enter Server IP : ");
        UartGetString(ip);

        UartPrintf("Enter Port : ");
        UartGetString(port);

        strcpy(buf, "AT+CIPSTART=");
        strcat(buf, "\"TCP\"");
        strcat(buf, ",");
        strcat(buf, "\"");
        strcat(buf, ip);
        strcat(buf, "\"");
        strcat(buf, ",");
        strcat(buf, port);
    }
    else if(cmd == AT_CIPSEND)
    {
        UartPrintf("Enter string to transmit : ");
        
        // payload prefix ('>')  ==> do not need
        //payload[0] = '>';
        
        // payload
        UartGetString(payload);
        strcat(payload, "\r\n");
        
        sprintf(buf, "%s%d", "AT+CIPSEND=", strlen(payload));
    }
    else if(cmd == AT_SLEEP)
    {
        sprintf(buf, "%s", "AT+SLEEP=2");
    }
    else if(cmd == AT_GSLP)
    {
        unsigned int time;
        UartPrintf("Enter time in miliseconds to enter deep sleep : ");
      
        time = UartGetIntNum();        
        sprintf(buf, "%s%d", "AT+GSLP=", time);

        UartPrintf("ESP8266 will wake up after %d miliseconds from now \r\n", time);
    }
    
    strcat(buf, "\r\n");
    HAL_UART_Transmit(huart, (uint8_t *)buf, strlen(buf), -1);
    
    // payload
    if(cmd == AT_CIPSEND)
    {
        HAL_Delay(50);
        HAL_UART_Transmit(huart, (uint8_t *)payload, strlen(payload), -1);
    }
}

void Esp8266_Reset()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

    
    HAL_Delay(2000);

    gReadPtrInRxDMABuffer = 0;
    HAL_UART_Receive_DMA(huart, gRxBuffer, sizeof(gRxBuffer));
    return;

    
    //
    // display the received data as binary form when ESP8266 reset
    //
     
    UartPrintf("========= ESP8266 RESET =========== \r\n");
    
    unsigned int next_tick = HAL_GetTick() + 5000;
    unsigned int n = 0;

    UartPrintf("%02x : ", n);
    while(HAL_GetTick() < next_tick)
    {
          unsigned char buf[16];
          unsigned int len = Esp8266_GetDataLengthInBuffer();
          
          if(len > sizeof(buf)) len = sizeof(buf);
          if(len)
          { 
              Esp8266_GetBytes(buf, len);
           
              for(int k=0; k<len; k++)
              {
                  if( (buf[k] >= 0x20) && (buf[k] <= 0x7F) )
                  {
                      UartPrintf("%02x(%c) ",buf[k], buf[k]);
                  }
                  else
                  {
                      UartPrintf("%02x(.) ",buf[k]);
                  }
                  
                  n = n + 1;
                  if( (n%16) == 0 )
                  {
                      UartPrintf("\r\n");
                      UartPrintf("%02x : ", n);
                  }
              }
          }
    }
}

void Esp8266_Flash(unsigned int enable)
{
    if(enable)  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    else        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}


void Esp8266_SendCmdTest(AT_CMD cmd)
{
    Esp8266_CmdHandler(cmd);
    if(Esp8266_WaitResponse() == RESPONSE_NO_RESPONSE)
    {
        UartPrintf("===> No Response. \r\n");
    }
}


void Esp8266Init(UART_HandleTypeDef * pUartHandle, DMA_HandleTypeDef * pUartDMARxHandle)
{
    huart = pUartHandle;
    hdma_uart_rx = pUartDMARxHandle;
      
    gReadPtrInRxDMABuffer = 0;
    HAL_UART_Receive_DMA(huart, gRxBuffer, sizeof(gRxBuffer));
    
//    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
    
//    __HAL_UART_ENABLE_IT(huart, UART_IT_LBD);
}


void Esp8266_ErrorHandler(unsigned int n)
{
    //UartPrintf("===> Reception Error [%d] \r\n", n);
    //while(1);
}

void Esp8266_Test()
{
      int i, sel;

      gTestFunc = (unsigned int)Esp8266_Test;
      
      while(1) 
      {
              i = 0;
              UartPrintf("+-------------------------------------------------+\r\n");		
              while(1) 
              {
                      UartPrintf("| %2d:%s \r\n",i+1,function[i][1]);
                      i++;
                      if((int)(function[i][0])==0) 
                      {
                              break;
                      }
              }
              UartPrintf("+-------------------------------------------------+\r\n");		
              UartPrintf(" Select the number to test : ");
              sel = UartGetIntNum();
              sel--;
              UartPrintf("+-------------------------------------------------+\r\n\n\n");		

              if(sel>=0 && sel<i )
              {
                      ( (void (*)(AT_CMD)) (function[sel][0]) )(sel);
              }
              else
              {
                      UartPrintf("Wrong number seleted.. Try again!! \r\n\n\n");
              }
      }
    
}
