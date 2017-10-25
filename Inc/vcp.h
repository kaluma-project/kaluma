#ifndef __VCP_H__
#define __VCP_H__

void VcpDebugInit(UART_HandleTypeDef *);
void VcpPrintf(const char *fmt,...);
char VcpIsCharReceived(void);
int VcpGetIntNum(void);

#endif
