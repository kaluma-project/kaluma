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

#include "rtc.h"

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

void km_rtc_init() {
  rtc_init();
  datetime_t t = {
      .year = 1970, .month = 1, .day = 1, .dotw = 4, .min = 0, .sec = 0};
  rtc_set_datetime(&t);
}

void km_rtc_cleanup() {}

void km_rtc_set_datetime(km_rtc_datetime_t* datetime) {
  datetime_t t;
  t.year = datetime->year;
  t.month = datetime->month;
  t.day = datetime->day;
  t.dotw = datetime->dotw;
  t.hour = datetime->hour;
  t.min = datetime->min;
  t.sec = datetime->sec;
  rtc_set_datetime(&t);
}

void km_rtc_get_datetime(km_rtc_datetime_t* datetime) {
  datetime_t t;
  rtc_get_datetime(&t);
  datetime->year = t.year;
  datetime->month = t.month;
  datetime->day = t.day;
  datetime->dotw = t.dotw;
  datetime->hour = t.hour;
  datetime->min = t.min;
  datetime->sec = t.sec;
}
