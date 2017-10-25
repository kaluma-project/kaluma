#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "mpu6050.h"

#define MPU6050_ADDR    (0x68 << 1)

I2C_HandleTypeDef * gphi2c1;


//
//
//
void Mpu6050_Init(I2C_HandleTypeDef * hi2c)
{
    gphi2c1 = hi2c;
}

void Mpu6050_Test()
{
    uint8_t buf[2];
    HAL_StatusTypeDef status;

    UartPrintf("MPU6050 Test Starts. \r\n");		

    buf[0] = 0;
    status = HAL_I2C_Mem_Write(gphi2c1, MPU6050_ADDR, 0x6B, 1, buf, 1, (uint32_t)-1);
    
    HAL_Delay(100);

    while(1)
    {
        status = HAL_I2C_Mem_Read(gphi2c1, MPU6050_ADDR, 0x3D, 1, buf, 2, (uint32_t)-1);
        unsigned short val = (buf[0] << 8) | buf[1];
        if(val > 32768) val = val - 65536;
        
        UartPrintf("status[%d], data[%d] \r\n", status, val);		
        
        
        HAL_Delay(1000);
    }
  
}
