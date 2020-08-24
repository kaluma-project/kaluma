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

#ifndef __ESP32_DEVC_H
#define __ESP32_DEVC_H

#include <sys/types.h>
#define KAMELEON_MANUFACTURER_STRING "Kameleon (kameleon.io)"
#define KAMELEON_PRODUCT_STRING "ESP32 DevC"
#define KAMELEON_SERIALNUMBER_STRING "00000000001A"

#define GPIO_NUM  32
#define ADC_NUM 6
#define PWM_NUM 6
#define I2C_NUM 2
#define SPI_NUM 2
#define UART_NUM 2
#define LED_NUM 0
#define BUTTON_NUM 1

#define APB1    0
#define APB2    1

#define ADC_RESOLUTION_BIT 12
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

#endif /* __ESP32_DEVC_H */
