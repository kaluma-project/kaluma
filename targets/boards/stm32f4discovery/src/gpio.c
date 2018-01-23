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
#include "stm32f4xx.h"
#include "stm32f4discovery.h"
#include "gpio.h"

extern struct {
    GPIO_TypeDef * port;
    uint32_t pin;
} gpio_port_pin[];

/** 
*/
void gpio_set_io_mode(uint8_t pin, gpio_io_mode_t mode) {
  assert_param(pin < GPIO_NUM);
  assert_param(mode==GPIO_IO_MODE_INPUT || mode==GPIO_IO_MODE_OUTPUT);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = gpio_port_pin[pin].pin;
  GPIO_InitStruct.Mode = (mode == GPIO_IO_MODE_INPUT) ?  GPIO_MODE_INPUT:GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio_port_pin[pin].port, &GPIO_InitStruct);   
}

/** 
*/
void gpio_write(uint8_t pin, uint8_t value) {
  assert_param(pin < GPIO_NUM);
  assert_param(value==GPIO_LOW || value==GPIO_HIGH);

  GPIO_PinState pin_state = (value == GPIO_LOW) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin, pin_state);
}

/** 
*/
uint8_t gpio_read(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  GPIO_PinState pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  return (pin_state == GPIO_PIN_RESET) ? GPIO_LOW : GPIO_HIGH;
}

/** 
*/
void gpio_toggle(uint8_t pin) {
  assert_param(pin < GPIO_NUM);
  HAL_GPIO_TogglePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
}

/** 
*/
void gpio_set_spi_mode(uint8_t pin) {
  assert_param(IS_GPIO_SPI_PIN(pin));

  // todo
}

/** 
*/
void gpio_spi_config(spi_clk_mode_t mode, uint32_t prescaler, uint32_t cs_pin) {
  uint32_t polarity, phase;

  if (mode == POLARITY_LOW_PHASE_1EDGE) {
    polarity = SPI_POLARITY_LOW;
    phase = SPI_PHASE_1EDGE;
  } else if (mode == POLARITY_LOW_PHASE_2EDGE) {
    polarity = SPI_POLARITY_LOW;
    phase = SPI_PHASE_2EDGE;
  } else if (mode == POLARITY_HIGH_PHASE_1EDGE) {
    polarity = SPI_POLARITY_HIGH;
    phase = SPI_PHASE_1EDGE;
  } else if (mode == POLARITY_HIGH_PHASE_2EDGE) {
    polarity = SPI_POLARITY_HIGH;
    phase = SPI_PHASE_2EDGE;
  }

  // todo
}


void gpio_test() {
#if 1
  gpio_set_io_mode(76, GPIO_IO_MODE_OUTPUT);
  gpio_set_io_mode(77, GPIO_IO_MODE_OUTPUT);
  gpio_set_io_mode(78, GPIO_IO_MODE_OUTPUT);
  gpio_set_io_mode(79, GPIO_IO_MODE_OUTPUT);

  while(1) {
    
    gpio_write(77, GPIO_HIGH);
    delay(1000);

    gpio_write(76, GPIO_HIGH);
    delay(1000);

    gpio_write(78, GPIO_HIGH);
    delay(1000);

    gpio_write(79, GPIO_HIGH);
    delay(1000);
    
    gpio_write(77, GPIO_LOW);
    delay(1000);

    gpio_write(76, GPIO_LOW);
    delay(1000);

    gpio_write(78, GPIO_LOW);
    delay(1000);

    gpio_write(79, GPIO_LOW);
    delay(1000);
  }

#else
  uint8_t pin_key = 18;
  gpio_set_io_mode(pin_key, GPIO_IO_MODE_INPUT);

  uint8_t pin_led = 17;
  gpio_set_io_mode(pin_led, GPIO_IO_MODE_OUTPUT);

  while(1) {
    
    uint8_t val = gpio_read(pin_key);
    
    if(val == GPIO_LOW) 
    {
      gpio_write(pin_led, GPIO_LOW);
      //tty_printf("key is pressed \r\n");
    }
    else
    {
      gpio_write(pin_led, GPIO_HIGH);
    }
  }
#endif  
}