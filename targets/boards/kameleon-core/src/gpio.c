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
#include "kameleon_core.h"
#include "gpio.h"

const struct {
    GPIO_TypeDef * port;
    uint32_t pin;
 } gpio_port_pin[] = {
   {GPIOB, GPIO_PIN_0},     // 0
   {GPIOB, GPIO_PIN_1},     // 1
   {GPIOA, GPIO_PIN_0},     // 2
   {GPIOA, GPIO_PIN_1},     // 3
   {GPIOA, GPIO_PIN_2},     // 4
   {GPIOA, GPIO_PIN_3},     // 5
   {GPIOA, GPIO_PIN_9},     // 6
   {GPIOA, GPIO_PIN_10},    // 7

   {GPIOB, GPIO_PIN_6},     // 8
   {GPIOB, GPIO_PIN_7},     // 9
   {GPIOA, GPIO_PIN_5},     // 10
   {GPIOA, GPIO_PIN_6},     // 11
   {GPIOA, GPIO_PIN_7},     // 12
   {GPIOB, GPIO_PIN_4},     // 13
   {GPIOB, GPIO_PIN_5},     // 14
   {GPIOB, GPIO_PIN_10},    // 15

   {GPIOB, GPIO_PIN_9},     // 16
   {GPIOB, GPIO_PIN_3},     // 17
   {GPIOB, GPIO_PIN_12},    // 18
   {GPIOC, GPIO_PIN_15},    // 19
   {GPIOB, GPIO_PIN_14},    // 20 (LED)
   {GPIOA, GPIO_PIN_4},     // 21 (BUTTON)
};

static uint32_t microseconds_cycle;

/**
 * Initialize all GPIO when system started
 */
void gpio_init() {
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12);
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_15);

  /** micro seconds timer init.
  */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  microseconds_cycle = HAL_RCC_GetHCLKFreq() / 1000000; /* 96 */
}

/**
 * Cleanup all GPIO when system cleanup
 */
void gpio_cleanup() {
  gpio_init();
}

/**
*/
int gpio_set_io_mode(uint8_t pin, gpio_io_mode_t mode) {
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  if ((mode != GPIO_IO_MODE_INPUT) && (mode != GPIO_IO_MODE_INPUT_PULLUP))
    mode = GPIO_IO_MODE_OUTPUT;
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = gpio_port_pin[pin].pin;
  GPIO_InitStruct.Mode = (mode == GPIO_IO_MODE_OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = (mode == GPIO_IO_MODE_INPUT_PULLUP) ? GPIO_PULLUP : GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio_port_pin[pin].port, &GPIO_InitStruct);
  return 0;
}

/**
*/
int gpio_write(uint8_t pin, uint8_t value) {
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  if (value != GPIO_LOW)
    value = GPIO_HIGH;
  GPIO_PinState pin_state = (value == GPIO_LOW) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin, pin_state);
  return 0;
}

/**
*/
int gpio_read(uint8_t pin) {
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  GPIO_PinState pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  return (pin_state == GPIO_PIN_RESET) ? GPIO_LOW : GPIO_HIGH;
}

/**
*/
int gpio_toggle(uint8_t pin) {
  if (pin >= GPIO_NUM)
    return GPIOPORT_ERROR;
  HAL_GPIO_TogglePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  return 0;
}


/**
 * Return MAX of the micro seconde counter 44739242
*/
__STATIC_INLINE uint32_t micro_maxtime() {
  return (0xFFFFFFFFU / microseconds_cycle);
}
/**
 * Return micro seconde counter
*/
__STATIC_INLINE uint32_t micro_gettime() {
  return (DWT->CYCCNT / microseconds_cycle);
}

/**
 * micro secoded delay
*/
void micro_delay(uint32_t usec) {
  uint32_t time_diff;
  uint32_t start = DWT->CYCCNT;
  do {
    uint32_t now = DWT->CYCCNT;
    if (now >= start)
      time_diff = now - start;
    else
      time_diff = (0xFFFFFFFFU - start) + now;
  } while (time_diff / microseconds_cycle < usec);
}

/**
 * make pulse on the GPIO pin.
*/
int pulse_write(uint8_t pin, uint8_t state, uint16_t *arr, uint8_t length) {
  uint8_t cnt;
  uint16_t delay;
  GPIO_PinState pin_state = (state == GPIO_LOW) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin, pin_state);
  for (cnt = 0; cnt < length; cnt++) {
    delay = arr[cnt];
    micro_delay(delay);
    HAL_GPIO_TogglePin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  }
  return 0;
}

uint8_t check_timeout(uint32_t start, uint32_t timeout) {
  uint32_t tout, now;
  now = micro_gettime();
  if (now >= start)
    tout = now - start;
  else
    tout = (micro_maxtime() - start) + now;
  if (tout > timeout)
    return 1;
  return 0;
}

/**
 * Read pulse signal on the GPIO pin.
*/
int pulse_read(uint8_t pin, uint8_t state, uint16_t *arr, uint8_t length, uint32_t timeout) {
  uint8_t cnt = 0;
  uint32_t start, last, now;
  GPIO_PinState pin_state;
  GPIO_PinState pre_pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  start = micro_gettime();
  while ((state < 2) && (pre_pin_state != state)) {
    if (check_timeout(start, timeout))
      return 0;
    pre_pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
  }
  start = micro_gettime();
  last = start;
  do {
    pin_state = HAL_GPIO_ReadPin(gpio_port_pin[pin].port, gpio_port_pin[pin].pin);
    now = micro_gettime();
    if (pin_state != pre_pin_state) {
      pre_pin_state = pin_state;
      if (now >= last)
        arr[cnt++] = now - last;
      else
        arr[cnt++] = (micro_maxtime() - last) + now;
      last = now;
    } else {
      if (check_timeout(start, timeout))
        return cnt;
    }
  } while (cnt < length);
  return cnt;
}
