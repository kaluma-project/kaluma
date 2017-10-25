#ifndef __ESP8266_H__
#define __ESP8266_H__

void Esp8266Init(UART_HandleTypeDef * pUartHandle, DMA_HandleTypeDef * pUartDMARxHandle);
void Esp8266_ErrorHandler(unsigned int);

#endif
