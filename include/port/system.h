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

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stdint.h>

#define NVIC_INT_CTRL_REG   (*(volatile uint32_t *)0xe000ed04)
#define NVIC_PENDSVSET_BIT  (1UL << 28UL)

extern const char system_arch[];
extern const char system_platform[];

/**
 * Initialize the system
 */
void system_init();

/**
 * Cleanup all resources in the system
 */
void system_cleanup();

/**
 * Increment tick count
 */
void inc_tick();

/**
 * Delay in milliseconds
 *
 * @param {uint64_t} msec
 */
void delay(uint64_t msec);

/**
 * Return current time (UNIX timestamp in milliseconds)
 */
uint64_t gettime();

/**
 * Set current time (UNIX timestamp in milliseconds)
 *
 * @param {uint64_t} msec
 */
void settime(uint64_t time);

/**
 * Pend SV Interrupt
 */
void SetPendSV();

/**
 * Firmware update request
 */
void request_firmup();
#endif /* __SYSTEM_H */
