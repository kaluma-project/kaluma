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
#include "kameleon_core.h"
#include "gpio.h"
#include <driver/gpio.h>

void gpio_init()
{
}

void gpio_cleanup()
{
}

int gpio_set_io_mode(uint8_t pin, gpio_io_mode_t mode)
{
  printf("gpio_set_io_mode(%d, %d)\n", pin,mode);
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << pin);
  io_conf.pull_down_en = 0;
  switch(mode){
  case GPIO_IO_MODE_INPUT:
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    break;
  case GPIO_IO_MODE_OUTPUT:
    io_conf.mode = GPIO_MODE_OUTPUT;
    break;
  case GPIO_IO_MODE_INPUT_PULLUP:
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    break;
  }
  gpio_config(&io_conf);
  return 0;
}

int gpio_write(uint8_t pin, uint8_t value)
{
  printf("gpio_write(%d,%d)\n", pin,value);
  gpio_set_level(pin, value);
  return 0;
}

int gpio_toggle(uint8_t pin)
{
  int oldVal = (GPIO_REG_READ(GPIO_OUT_REG)  >> pin) & 1U;
  int newVal = !oldVal;
  printf("gpio_toggle(%d) old:%d, new:%d\n", pin, oldVal, newVal);
  gpio_set_level(pin, newVal);
  return newVal;
}

int gpio_read(uint8_t pin)
{
  int ret= gpio_get_level(pin);
  printf("gpio_read(%d): %d\n", pin, ret);
  return ret;
}

