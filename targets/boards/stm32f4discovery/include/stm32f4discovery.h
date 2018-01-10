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

#ifndef __STM32F4DISCOVERY_H
#define __STM32F4DISCOVERY_H

#include "stm32f4xx.h"

#define PLL_M 8
#define PLL_N 336
#define PLL_Q 7

#define FLASH_SIZE (1024 * 1024)
#define FLASH_BASE_ADDR (0x08000000)

#define SRAM_SIZE (128 * 1024)
#define SRAM_BASE_ADDR (0x20000000)

#define CCM_SIZE (64 * 1024)
#define CCM_BASE_ADDR (0x10000000)

#define GPIO_NUM  80

#define APB1    0
#define APB2    1

#define NUM_ADC_CHANNEL 8
#define ADC_RESOLUTION_BIT 12

#define IS_PWM_PINS(PIN) (((PIN) == 0) || \
                         ((PIN) == 2) || \
                         ((PIN) == 4) || \
                         ((PIN) == 14) || \
                         ((PIN) == 15) || \
                         ((PIN) == 18) || \
                         ((PIN) == 35) || \
                         ((PIN) == 45))

#define IS_ADC_PINS(PIN) (((PIN) == 23) || \
                         ((PIN) == 25) || \
                         ((PIN) == 28) || \
                         ((PIN) == 29) || \
                         ((PIN) == 30) || \
                         ((PIN) == 31) || \
                         ((PIN) == 69) || \
                         ((PIN) == 70))

#endif /* __STM32F4DISCOVERY_H */
