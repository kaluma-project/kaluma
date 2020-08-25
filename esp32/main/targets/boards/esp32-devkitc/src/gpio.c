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

#include <stdint.h>
#include "esp32_devkitc.h"
#include "gpio.h"
#include <driver/gpio.h>

const gpio_num_t gpio_port_pin[] = {
   GPIO_NUM_23,
   GPIO_NUM_22,
   GPIO_NUM_1,
   GPIO_NUM_3,
   GPIO_NUM_21,
   GPIO_NUM_19,
   GPIO_NUM_18,
   GPIO_NUM_5,
   GPIO_NUM_17,
   GPIO_NUM_16,
   GPIO_NUM_4,
   GPIO_NUM_0, // Button
   GPIO_NUM_2,
   GPIO_NUM_15,
   GPIO_NUM_8,
   GPIO_NUM_7,
   GPIO_NUM_6,
   GPIO_NUM_11,
   GPIO_NUM_10,
   GPIO_NUM_9,
   GPIO_NUM_13,
   GPIO_NUM_12,
   GPIO_NUM_14,
   GPIO_NUM_27,
   GPIO_NUM_26,
   GPIO_NUM_25,
   GPIO_NUM_33,
   GPIO_NUM_32,
   GPIO_NUM_35, // Input only
   GPIO_NUM_34, // Input only
   GPIO_NUM_39, // Input only
   GPIO_NUM_36  // Input only
};

void gpio_init()
{
#if 0 // TODO: disable due to the system halt
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.pin_bit_mask = 0xFF0EEFFFFFULL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  io_conf.mode = GPIO_MODE_INPUT;
  gpio_config(&io_conf);
#endif
}

void gpio_cleanup()
{
  gpio_init();
}

int gpio_set_io_mode(uint8_t pin, gpio_io_mode_t mode)
{
  gpio_config_t io_conf;
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  if ((mode != GPIO_IO_MODE_INPUT) && (mode != GPIO_IO_MODE_INPUT_PULLUP))
    mode = GPIO_IO_MODE_OUTPUT;
  //printf("gpio_set_io_mode(%d, %d)\n", gpio_port_pin[pin],mode);
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << gpio_port_pin[pin]);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  io_conf.mode = GPIO_MODE_INPUT;
  switch(mode) {
  case GPIO_IO_MODE_INPUT:
    break;
  case GPIO_IO_MODE_OUTPUT:
    io_conf.mode = GPIO_MODE_OUTPUT;
    break;
  case GPIO_IO_MODE_INPUT_PULLUP:
    io_conf.pull_up_en = 1;
    break;
  }
  gpio_config(&io_conf);
  return 0;
}

int gpio_write(uint8_t pin, uint8_t value)
{
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  if (value != GPIO_LOW)
    value = GPIO_HIGH;
  printf("gpio_write(%d,%d)\n", gpio_port_pin[pin],value);
  gpio_set_level(gpio_port_pin[pin], value);
  return 0;
}

int gpio_toggle(uint8_t pin)
{
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  int oldVal = (GPIO_REG_READ(GPIO_OUT_REG) >> gpio_port_pin[pin]) & 1U;
  int newVal = !oldVal;
  printf("gpio_toggle(%d) old:%d, new:%d\n", gpio_port_pin[pin], oldVal, newVal);
  gpio_set_level(gpio_port_pin[pin], newVal);
  return newVal;
}

int gpio_read(uint8_t pin)
{
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  int ret= gpio_get_level(gpio_port_pin[pin]);
  printf("gpio_read(%d): %d\n", gpio_port_pin[pin], ret);
  return ret;
}

