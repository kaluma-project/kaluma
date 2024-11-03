/* Copyright (c) 2017 Kaluma
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

#include "i2c.h"

#include "board.h"
#include "err.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "system.h"

static struct __i2c_status_s {
  km_i2c_mode_t mode;
} __i2c_status[KALUMA_I2C_NUM];

static bool __check_i2c_pins(uint8_t bus, km_i2c_pins_t pins) {
  if ((pins.sda > 27) || (pins.scl > 27)) {
    return false;
  }
  if (bus == 0) {
    if (((pins.sda >= 0) && ((pins.sda % 4) != 0)) || (pins.sda > 21)) {
      return false;
    }
    if (((pins.scl >= 0) && ((pins.scl % 4) != 1)) || (pins.scl > 21)) {
      return false;
    }
  } else if (bus == 1) {
    if (((pins.sda >= 0) && ((pins.sda % 4) != 2)) ||
        ((pins.sda > 21) && (pins.sda < 26))) {
      return false;
    }
    if (((pins.scl >= 0) && ((pins.scl % 4) != 3)) ||
        ((pins.scl > 21) && (pins.scl < 26))) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

/**
 * Return default I2C pins. -1 means there is no default value on that pin.
 */
km_i2c_pins_t km_i2c_get_default_pins(uint8_t bus) {
  km_i2c_pins_t pins;
  if (bus == 0) {
    pins.sda = 4;
    pins.scl = 5;
  } else if (bus == 1) {
    pins.sda = 2;
    pins.scl = 3;
  } else {
    pins.sda = -1;
    pins.scl = -1;
  }
  return pins;
}

/**
 * Initialize all I2C when system started
 */
void km_i2c_init() {
  for (int i = 0; i < KALUMA_I2C_NUM; i++) {
    __i2c_status[i].mode = KM_I2C_NONE;
  }
}

/**
 * Cleanup all I2C when system cleanup
 */
void km_i2c_cleanup() {
  i2c_deinit(i2c0);
  i2c_deinit(i2c1);
  km_i2c_init();
}

static i2c_inst_t *__get_i2c_no(uint8_t bus) {
  if (bus == 0) {
    return i2c0;
  } else if (bus == 1) {
    return i2c1;
  } else {
    return NULL;
  }
}

int km_i2c_setup_master(uint8_t bus, uint32_t speed, km_i2c_pins_t pins) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_NONE) ||
      (__check_i2c_pins(bus, pins) == false)) {
    return EDEVINIT;
  }
  __i2c_status[bus].mode = KM_I2C_MASTER;
  if (speed > I2C_MAX_CLOCK) {
    speed = I2C_MAX_CLOCK;
  }
  if (pins.sda >= 0) {
    gpio_set_function(pins.sda, GPIO_FUNC_I2C);
    gpio_pull_up(pins.sda);
  }
  if (pins.scl >= 0) {
    gpio_set_function(pins.scl, GPIO_FUNC_I2C);
    gpio_pull_up(pins.scl);
  }
  i2c_init(i2c, speed);
  return 0;
}

int km_i2c_setup_slave(uint8_t bus, uint8_t address, km_i2c_pins_t pins) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_NONE) ||
      (__check_i2c_pins(bus, pins) == false)) {
    return EDEVINIT;
  }
  __i2c_status[bus].mode = KM_I2C_SLAVE;
  return 0;
}

int km_i2c_mem_write_master(uint8_t bus, uint8_t address, uint16_t mem_addr,
                            uint8_t mem_addr_size, uint8_t *buf, size_t len,
                            uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  int ret;
  uint8_t __memaddr[2];
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_MASTER)) {
    return EDEVWRITE;
  }
  if (mem_addr_size == 16) {  // 16 bit mem address
    __memaddr[0] = ((mem_addr >> 8) & 0xFF);
    __memaddr[1] = (mem_addr & 0xFF);
    ret = i2c_write_blocking(i2c, address, __memaddr, 2, true);
  } else {  // 8 bit mem address
    __memaddr[0] = (mem_addr & 0xFF);
    ret = i2c_write_blocking(i2c, address, __memaddr, 1, true);
  }
  if (ret >= 0) {
    ret = i2c_write_timeout_us(i2c, address, buf, len, false, timeout * 1000);
  }
  if (ret < 0) {
    return EDEVWRITE;
  }
  return ret;
}

int km_i2c_mem_read_master(uint8_t bus, uint8_t address, uint16_t mem_addr,
                           uint8_t mem_addr_size, uint8_t *buf, size_t len,
                           uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  int ret;
  uint8_t __memaddr[2];
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_MASTER)) {
    return EDEVREAD;
  }
  if (mem_addr_size == 16) {  // 16 bit mem address
    __memaddr[0] = ((mem_addr >> 8) & 0xFF);
    __memaddr[1] = (mem_addr & 0xFF);
    ret = i2c_write_blocking(i2c, address, __memaddr, 2, true);
  } else {  // 8 bit mem address
    __memaddr[0] = (mem_addr & 0xFF);
    ret = i2c_write_blocking(i2c, address, __memaddr, 1, true);
  }
  if (ret >= 0) {
    ret = i2c_read_timeout_us(i2c, address, buf, len, false, timeout * 1000);
  }
  if (ret < 0) {
    return EDEVREAD;
  }
  return ret;
}

int km_i2c_write_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len,
                        uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  int ret;
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_MASTER)) {
    return EDEVWRITE;
  }
  ret = i2c_write_timeout_us(i2c, address, buf, len, false, timeout * 1000);
  if (ret < 0) {
    return EDEVWRITE;
  }
  return ret;
}

int km_i2c_write_slave(uint8_t bus, uint8_t *buf, size_t len,
                       uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_SLAVE)) {
    return EDEVWRITE;
  }
  return 0;
}

int km_i2c_read_master(uint8_t bus, uint8_t address, uint8_t *buf, size_t len,
                       uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  int ret;
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_MASTER)) {
    return EDEVREAD;
  }
  ret = i2c_read_timeout_us(i2c, address, buf, len, false, timeout * 1000);
  if (ret < 0) {
    return EDEVREAD;
  }
  return ret;
}

int km_i2c_read_slave(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  if ((i2c == NULL) || (__i2c_status[bus].mode != KM_I2C_SLAVE)) {
    return EDEVREAD;
  }
  return 0;
}

int km_i2c_close(uint8_t bus) {
  i2c_inst_t *i2c = __get_i2c_no(bus);
  if ((i2c == NULL) || (__i2c_status[bus].mode == KM_I2C_NONE)) {
    return EDEVINIT;
  }
  i2c_deinit(i2c);
  __i2c_status[bus].mode = KM_I2C_NONE;
  return 0;
}
