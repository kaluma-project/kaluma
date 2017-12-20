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
#include "stm32f4xx_hal.h"
#include "gpio.h"

/**
 * Initialze GPIO pins
 * ~ : means PWM (e.g. ~4, ~5, ~6...)
 * @ : means analog (e.g. @1, @2, @3, ...)
 */
static struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOA, GPIO_PIN_1},     // 0
   {GPIOA, GPIO_PIN_4},     // 1
   {GPIOA, GPIO_PIN_5},     // 2
   {GPIOA, GPIO_PIN_6},     // 3
   {GPIOA, GPIO_PIN_7},     // 4
   {GPIOA, GPIO_PIN_9},     // 5
   {GPIOA, GPIO_PIN_10},    // 6
   {GPIOA, GPIO_PIN_15},    // 7

   {GPIOB, GPIO_PIN_3},     // 8
   {GPIOB, GPIO_PIN_4},     // 9
   {GPIOB, GPIO_PIN_6},     // 10
   {GPIOB, GPIO_PIN_7},     // 11
   {GPIOC, GPIO_PIN_0},     // 12
   {GPIOC, GPIO_PIN_1},     // 13
   {GPIOC, GPIO_PIN_2},     // 14
   {GPIOC, GPIO_PIN_3},     // 15

   {GPIOA, GPIO_PIN_0},     // 16
   {GPIOB, GPIO_PIN_13},    // 17 (LED)
   {GPIOC, GPIO_PIN_8},     // 18 (KEY)
};

/** 
*/
void gpio_set_io_mode(uint8_t pin, gpio_io_mode_t mode) {
  GPIO_InitTypeDef GPIO_InitStruct;
  
  assert_param(pin < GPIO_NUM);
  assert_param(mode==GPIO_IO_MODE_INPUT || mode==GPIO_IO_MODE_OUTPUT);

  GPIO_InitStruct.Pin = gpio_port_pin[pin].pin;
  GPIO_InitStruct.Mode = (mode == GPIO_IO_MODE_INPUT) ?  GPIO_MODE_INPUT:GPIO_MODE_OUTPUT_PP;

  if (mode==GPIO_IO_MODE_OUTPUT) {
    GPIO_InitStruct.Pull = GPIO_PULLUP;
  } else {
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    /* if the pin is the key (should be modified in the future) */
    if (pin == 18) {
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    } 
  }   
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
#if 0
  uint8_t pin = 17;
  gpio_set_io_mode(pin, GPIO_IO_MODE_OUTPUT);

  while(1) {
    
    gpio_write(pin, GPIO_HIGH);
    delay(1000);

    gpio_write(pin, GPIO_LOW);
    delay(1000);

    gpio_toggle(pin);
    delay(1000);

    gpio_toggle(pin);
    delay(1000);

    assert_param(1);
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