#ifndef __MPU6050_H__
#define __MPU6050_H__


void Mpu6050_Init(I2C_HandleTypeDef * hi2c);
void Mpu6050_Test(void);

#endif
