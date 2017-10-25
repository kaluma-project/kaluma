#ifndef __UART_H__
#define __UART_H__

void UartDebugInit(UART_HandleTypeDef *);
void UartPrintf(const char *fmt,...);
char UartIsCharReceived(void);
void UartGetString(char *);
void UartSendByte(char);
int UartGetIntNum(void);

#endif
