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

#include "kameleon_core.h"
#include "system.h"
#include "i2c.h"
#include "tty.h"

static I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef hi2c2;
static I2C_HandleTypeDef * handle[] = {&hi2c1, &hi2c2};
static I2C_TypeDef * instance[] = {I2C1, I2C2};

int i2c_setup_master(uint8_t bus, uint32_t speed) {
  assert_param(bus==0 || bus==1);

  handle[bus]->Instance = instance[bus];
  handle[bus]->Init.ClockSpeed = speed;
  handle[bus]->Init.DutyCycle = I2C_DUTYCYCLE_2;
  handle[bus]->Init.OwnAddress1 = 0;
  handle[bus]->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  handle[bus]->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  handle[bus]->Init.OwnAddress2 = 0;
  handle[bus]->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  handle[bus]->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  HAL_StatusTypeDef hal_status = HAL_I2C_Init(handle[bus]);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

int i2c_setup_slave(uint8_t bus, uint8_t address) {
  assert_param(bus==0 || bus==1);

  handle[bus]->Instance = instance[bus];
  handle[bus]->Init.ClockSpeed = 100000;
  handle[bus]->Init.DutyCycle = I2C_DUTYCYCLE_2;
  handle[bus]->Init.OwnAddress1 = (address << 1);
  handle[bus]->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  handle[bus]->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  handle[bus]->Init.OwnAddress2 = 0;
  handle[bus]->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  handle[bus]->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  HAL_StatusTypeDef hal_status = HAL_I2C_Init(handle[bus]);
  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}

int i2c_memWrite_master(uint8_t bus, uint8_t address, uint16_t memAddress, uint8_t memAdd16bit, uint8_t *buf, size_t len, uint32_t timeout) {
  uint16_t memAddSize;
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  if (memAdd16bit == 0)
    memAddSize = I2C_MEMADD_SIZE_8BIT;
  else
    memAddSize = I2C_MEMADD_SIZE_16BIT;
  hal_status = HAL_I2C_Mem_Write(handle[bus], address << 1, memAddress, memAddSize, buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_memRead_master(uint8_t bus, uint8_t address, uint16_t memAddress, uint8_t memAdd16bit, uint8_t *buf, size_t len, uint32_t timeout) {
  uint16_t memAddSize;
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  if (memAdd16bit == 0)
    memAddSize = I2C_MEMADD_SIZE_8BIT;
  else
    memAddSize = I2C_MEMADD_SIZE_16BIT;
  hal_status = HAL_I2C_Mem_Read(handle[bus], address << 1, memAddress, memAddSize, buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_write_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len, uint32_t timeout) {
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  hal_status = HAL_I2C_Master_Transmit(handle[bus], address << 1, buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_write_slave(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  /* in the case of slave mode, the parameter address is ignored. */
  hal_status = HAL_I2C_Slave_Transmit(handle[bus], buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_read_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len, uint32_t timeout) {
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  /* in the case of master mode */
  hal_status = HAL_I2C_Master_Receive(handle[bus], address << 1, buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_read_slave(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  HAL_StatusTypeDef hal_status;

  assert_param(bus==0 || bus==1);
  /* in the case of slave mode, the parameter address is ignored. */
  hal_status = HAL_I2C_Slave_Receive(handle[bus], buf, len, timeout);

  if (hal_status == HAL_OK) {
    return len;
  } else {
    return -1;
  }
}

int i2c_close(uint8_t bus) {
  assert_param(bus==0 || bus==1);
  HAL_StatusTypeDef hal_status = HAL_I2C_DeInit(handle[bus]);

  if (hal_status == HAL_OK) {
    return 0;
  } else {
    return -1;
  }
}
