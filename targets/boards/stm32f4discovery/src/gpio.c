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

/**
 * Initialze GPIO pins
 * ~ : means PWM (e.g. ~4, ~5, ~6...)
 * @ : means analog (e.g. @1, @2, @3, ...)
 */
static struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOC, GPIO_PIN_6},     // 0
   {GPIOC, GPIO_PIN_8},     // 1
   {GPIOC, GPIO_PIN_9},     // 2
   {GPIOA, GPIO_PIN_8},     // 3
   {GPIOA, GPIO_PIN_15},    // 4
   {GPIOC, GPIO_PIN_11},    // 5
   {GPIOD, GPIO_PIN_0},     // 6
   {GPIOD, GPIO_PIN_1},     // 7

   {GPIOD, GPIO_PIN_2},     // 8
   {GPIOD, GPIO_PIN_3},     // 9
   {GPIOD, GPIO_PIN_6},     // 10
   {GPIOD, GPIO_PIN_7},     // 11
   {GPIOB, GPIO_PIN_4},     // 12
   {GPIOB, GPIO_PIN_5},     // 13
   {GPIOB, GPIO_PIN_7},     // 14
   {GPIOB, GPIO_PIN_8},     // 15

   {GPIOE, GPIO_PIN_2},     // 16
   {GPIOE, GPIO_PIN_4},     // 17
   {GPIOE, GPIO_PIN_5},     // 18
   {GPIOE, GPIO_PIN_6},     // 19
   {GPIOC, GPIO_PIN_13},    // 20
   {GPIOC, GPIO_PIN_14},    // 21
   {GPIOC, GPIO_PIN_15},    // 22
   {GPIOC, GPIO_PIN_1},     // 23
   
   {GPIOC, GPIO_PIN_2},     // 24
   {GPIOA, GPIO_PIN_1},     // 25
   {GPIOA, GPIO_PIN_3},     // 26
   {GPIOA, GPIO_PIN_2},     // 27
   {GPIOC, GPIO_PIN_5},     // 28
   {GPIOC, GPIO_PIN_4},     // 29
   {GPIOB, GPIO_PIN_1},     // 30
   {GPIOB, GPIO_PIN_0},     // 31
   
   {GPIOB, GPIO_PIN_2},     // 32
   {GPIOE, GPIO_PIN_7},     // 33
   {GPIOE, GPIO_PIN_8},     // 34
   {GPIOE, GPIO_PIN_9},     // 35
   {GPIOE, GPIO_PIN_10},    // 36
   {GPIOE, GPIO_PIN_11},    // 37
   {GPIOE, GPIO_PIN_12},    // 38
   {GPIOE, GPIO_PIN_13},    // 39

   {GPIOE, GPIO_PIN_14},    // 40
   {GPIOE, GPIO_PIN_15},    // 41
   {GPIOB, GPIO_PIN_11},    // 42
   {GPIOB, GPIO_PIN_12},    // 43
   {GPIOB, GPIO_PIN_13},    // 44
   {GPIOB, GPIO_PIN_14},    // 45
   {GPIOB, GPIO_PIN_15},    // 46
   {GPIOD, GPIO_PIN_8},     // 47
   
   {GPIOD, GPIO_PIN_9},     // 48
   {GPIOD, GPIO_PIN_10},    // 49
   {GPIOD, GPIO_PIN_11},    // 50 
   {GPIOC, GPIO_PIN_7},     // 51 (START OF USED PINS)
   {GPIOA, GPIO_PIN_9},     // 52
   {GPIOA, GPIO_PIN_10},    // 53
   {GPIOA, GPIO_PIN_13},    // 54
   {GPIOA, GPIO_PIN_14},    // 55
   
   {GPIOC, GPIO_PIN_10},    // 56
   {GPIOC, GPIO_PIN_12},    // 57
   {GPIOD, GPIO_PIN_4},     // 58
   {GPIOD, GPIO_PIN_5},     // 59
   {GPIOB, GPIO_PIN_3},     // 60
   {GPIOB, GPIO_PIN_6},     // 61
   {GPIOB, GPIO_PIN_9},     // 62
   {GPIOE, GPIO_PIN_0},     // 63
   
   {GPIOE, GPIO_PIN_1},     // 64
   {GPIOE, GPIO_PIN_3},     // 65
   {GPIOH, GPIO_PIN_0},     // 66
   {GPIOH, GPIO_PIN_1},     // 67
   {GPIOC, GPIO_PIN_0},     // 68
   {GPIOC, GPIO_PIN_3},     // 69
   {GPIOA, GPIO_PIN_0},     // 70 (BUTTON)
   {GPIOA, GPIO_PIN_5},     // 71
   
   {GPIOA, GPIO_PIN_4},     // 72
   {GPIOA, GPIO_PIN_7},     // 73
   {GPIOA, GPIO_PIN_6},     // 74
   {GPIOB, GPIO_PIN_10},    // 75
   {GPIOD, GPIO_PIN_12},    // 76 (LED4)
   {GPIOD, GPIO_PIN_13},    // 77 (LED3)
   {GPIOD, GPIO_PIN_14},    // 78 (LED5)
   {GPIOD, GPIO_PIN_15},    // 79 (LED6)
};

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