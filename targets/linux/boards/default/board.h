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

#ifndef __LINUX_H
#define __LINUX_H

#include "jerryscript.h"

#define KALUMA_SYSTEM_ARCH "x86"
#define KALUMA_SYSTEM_PLATFORM "linux"

// binary (1008KB)
#define KALUMA_BINARY_MAX 0x100000

// flash (B + C + D = 1040KB (=16KB + 1024KB))
#define KALUMA_FLASH_OFFSET KALUMA_BINARY_MAX
#define KALUMA_FLASH_SECTOR_SIZE 4096
#define KALUMA_FLASH_SECTOR_COUNT 0
#define KALUMA_FLASH_PAGE_SIZE 256

// user program on flash (512KB)
#define KALUMA_PROG_SECTOR_BASE 4
#define KALUMA_PROG_SECTOR_COUNT 128

// #define KALUMA_GPIO_COUNT 22
// #define ADC_NUM 6
// #define PWM_NUM 6
// #define I2C_NUM 2
// #define SPI_NUM 2
// #define UART_NUM 2
// #define LED_NUM 1
// #define BUTTON_NUM 1

void board_init();

#endif /* __LINUX_H */
