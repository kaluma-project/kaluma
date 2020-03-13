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

#ifndef __KAMELEON_CORE_H
#define __KAMELEON_CORE_H

#include <stdint.h>

#define KAMELEON_MANUFACTURER_STRING ""
#define KAMELEON_PRODUCT_STRING "ESP32-WROVER-B"
#define KAMELEON_SERIALNUMBER_STRING "00000000001A"

#define PLL_M 8
#define PLL_N 192
#define PLL_Q 4

/*
#define FLASH_SIZE (512 * 1024)
#define FLASH_BASE_ADDR (0x08000000)

#define SRAM_SIZE (128 * 1024)
#define SRAM_BASE_ADDR (0x20000000)

#define GPIO_NUM  22
#define ADC_NUM 6
#define PWM_NUM 6
#define I2C_NUM 2
#define SPI_NUM 2
#define UART_NUM 2
#define LED_NUM 1
#define BUTTON_NUM 1

#define APB1    0
#define APB2    1

#define ADC_RESOLUTION_BIT 12
*/
/**
 * Error handler for the system driver error.
 */
void _Error_Handler(char * file, uint32_t line);

/**
 * this function is called in the pendable interrupt service routine which has
 * lowest priority to allow other interrupts service.
 */
void tty_transmit_data();

/**
 * return tx data length
 */
uint32_t tty_get_tx_data_length();

uint32_t tty_fill_rx_bytes(uint8_t * buf, uint32_t nToWrite);

#endif /* __KAMELEON_CORE_H */
