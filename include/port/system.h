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
 * Return MAX of the micro seconde counter
 * Use this value to detect counter overflow
*/
uint32_t micro_maxtime(void);

/**
 * Return micro seconde counter
*/
uint32_t micro_gettime(void);

/**
 * micro secoded delay
*/
void micro_delay(uint32_t usec);

/**
 * Firmware update request
 */
void request_firmup();

/**
 * check script running mode - skipping or running user script
 */
uint8_t running_script_check();

#endif /* __SYSTEM_H */
