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

#ifndef __KM_RTC_H
#define __KM_RTC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t year;  // 0 ~ ...
  uint8_t month;  // 1 ~ 12 (0 = january)
  uint8_t day;    // 1 ~ 31
  uint8_t dotw;   // 0 ~ 6 (0 = sunday)
  uint8_t hour;   // 0 ~ 23
  uint8_t min;    // 0 ~ 59
  uint8_t sec;    // 0 ~ 59
} km_rtc_datetime_t;

void km_rtc_init();
void km_rtc_cleanup();
void km_rtc_set_datetime(km_rtc_datetime_t *datetime);
void km_rtc_get_datetime(km_rtc_datetime_t *datetime);

#endif /* __KM_RTC_H */
