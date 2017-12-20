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

#include "stm32f4xx.h"
#include "i2c.h"

static i2c_mode_t mode;
static I2C_HandleTypeDef hi2c1;

/**
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {

  GPIO_InitTypeDef GPIO_InitStruct;
  if (hi2c->Instance==I2C1) {
    /**I2C1 GPIO Configuration    
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  }
}

/**
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c) {

  if (hi2c->Instance==I2C1) {
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();
  
    /**I2C1 GPIO Configuration    
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
  }
}

/**
 */
int i2c_write(uint8_t bus, uint8_t address, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0);
  HAL_StatusTypeDef hal_status;
  
  if (mode==I2C_MODE_MASTER) {
    /* in the case of master mode */
    hal_status = HAL_I2C_Master_Transmit(&hi2c1, address, buf, len, timeout);
  } else {
    /* in the case of slave mode, the parameter address is ignored. */
    hal_status = HAL_I2C_Slave_Transmit(&hi2c1, buf, len, timeout);
  }
  
  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

/**
 */
int i2c_write_char(uint8_t bus, uint8_t address, uint8_t ch, uint32_t timeout) {
  assert_param(bus==0);
  HAL_StatusTypeDef hal_status;
  
  if (mode==I2C_MODE_MASTER) {
    /* in the case of master mode */
    hal_status = HAL_I2C_Master_Transmit(&hi2c1, address, &ch, 1, timeout);
  } else {
    /* in the case of slave mode, the parameter address is ignored. */
    hal_status = HAL_I2C_Slave_Transmit(&hi2c1, &ch, 1, timeout);
  }
  
  if (hal_status == HAL_OK) {
    return 1;
  } else {
    return -1;
  }
}

/**
 */
int i2c_read(uint8_t bus, uint8_t address, uint8_t *buf, uint32_t len, uint32_t timeout) {
  assert_param(bus==0);
  HAL_StatusTypeDef hal_status;

  if (mode==I2C_MODE_MASTER) {
    /* in the case of master mode */
    hal_status = HAL_I2C_Master_Receive(&hi2c1, address, buf, len, timeout);
  } else {
    /* in the case of slave mode, the parameter address is ignored. */
    hal_status = HAL_I2C_Slave_Receive(&hi2c1, buf, len, timeout);
  }

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }  
}

/**
 */
int i2c_read_char(uint8_t bus, uint8_t address, uint32_t timeout) {
  assert_param(bus==0);
  uint8_t ch;
  HAL_StatusTypeDef hal_status;

  if (mode==I2C_MODE_MASTER) {
    /* in the case of master mode */
    hal_status = HAL_I2C_Master_Receive(&hi2c1, address, &ch, 1, timeout);
  } else {
    /* in the case of slave mode, the parameter address is ignored. */
    hal_status = HAL_I2C_Slave_Receive(&hi2c1, &ch, 1, timeout);
  }
 
  if (hal_status == HAL_OK) {
    return (ch & 0x0ff);
  } else {
    return -1;
  }
}
  
/**
 */
int i2c_close(uint8_t bus) {
  assert_param(bus==0);
  HAL_StatusTypeDef hal_status = HAL_I2C_DeInit(&hi2c1);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

/**
 */
int i2c_open_master(uint8_t bus) {
  assert_param(bus==0);
  mode = I2C_MODE_MASTER;

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  

  HAL_StatusTypeDef hal_status = HAL_I2C_Init(&hi2c1);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

/**
 */
int i2c_open_slave(uint8_t bus, uint8_t address) {
  assert_param(bus==0);
  mode = I2C_MODE_SLAVE;

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = (address << 1);
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  

  HAL_StatusTypeDef hal_status = HAL_I2C_Init(&hi2c1);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

//
//
//
void i2c_test() {
  i2c_open_master(0);
}