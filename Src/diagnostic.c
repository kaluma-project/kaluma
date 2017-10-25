#include "stm32f4xx_hal.h"
#include "main.h"
#include "vcp.h"

extern void Is25Lq_Test();
extern void Esp8266_Test();
extern void Mpu6050_Test();

static const void *function[][2]=
{
	(void *)Is25Lq_Test,	        "SPI Flash(IS25LQ040B) Test",
	(void *)Esp8266_Test,	        "ESP8266 12E Test",
	(void *)Mpu6050_Test,	        "MPU6050 (I2C) Test",
	0,0
};


void Diagnostic()
{
      int i, sel;

      //
      //
      //
      VcpPrintf("\r\n");
      VcpPrintf("*****************************************************************\r\n");
      VcpPrintf("*                                                               *\r\n");
      VcpPrintf("*           KAMELEON Diagnostic Firmware Testing                *\r\n");
      VcpPrintf("*                                                               *\r\n");
      VcpPrintf("*****************************************************************\r\n");
      
      while(1) 
      {
              i = 0;
              VcpPrintf("+-------------------------------------------------+\r\n");		
              while(1) 
              {
                      VcpPrintf("| %2d:%s \r\n",i+1,function[i][1]);
                      i++;
                      if((int)(function[i][0])==0) 
                      {
                              break;
                      }
              }
              VcpPrintf("+-------------------------------------------------+\r\n");		
              VcpPrintf(" Select the number to test : ");
              sel = VcpGetIntNum();
              sel--;
              VcpPrintf("+-------------------------------------------------+\r\n\n\n");		

              if(sel>=0 && sel<i )
              {
                      ( (void (*)(void)) (function[sel][0]) )();
              }
              else
              {
                      VcpPrintf("Wrong number seleted.. Try again!! \r\n\n\n");
              }
      }
}



