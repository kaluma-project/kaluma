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

#include "system.h"
#include "stm32f4xx_hal.h"
#include "system_low_level.h"

static uint64_t tick_count;

/** increment system timer tick every 1msec
*/
void inc_tick()
{
  tick_count++;
}

/** Kameleon Hardware System Initializations
*/
void system_init() {
  HAL_Init();
  SystemClock_Config();
  GpioClock_Config();
  Led_Config();
  SpiFlash_Config();
  UsbDevice_Config();
}

void delay(uint64_t msec) {
  HAL_Delay(msec);
}

uint64_t gettime() {
  return tick_count;
}

void settime(uint64_t time) {
  __set_PRIMASK(1);
 tick_count = time;
  __set_PRIMASK(0);
}
