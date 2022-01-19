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

#include <time.h>

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

void km_rtc_init() {
  rtc_init();
  // set as unix epoch time
  datetime_t t = {
      .year = 1970, .month = 1, .day = 1, .dotw = 4, .min = 0, .sec = 0};
  rtc_set_datetime(&t);
}

void km_rtc_cleanup() {}

void km_rtc_set_time(uint64_t time) {
  struct tm* ptm;
  uint64_t stime = time / 1000;
  // uint64_t ms = time % 1000;
  time_t t = (time_t)stime;
  ptm = gmtime(&t);
  datetime_t datetime;
  datetime.sec = ptm->tm_sec;
  datetime.min = ptm->tm_min;
  datetime.hour = ptm->tm_hour;
  datetime.day = ptm->tm_mday;
  datetime.dotw = ptm->tm_wday;
  datetime.month = ptm->tm_mon + 1;
  datetime.year = ptm->tm_year + 1900;
  rtc_set_datetime(&datetime);
}

uint64_t km_rtc_get_time() {
  datetime_t datetime;
  rtc_get_datetime(&datetime);
  struct tm ts;
  ts.tm_sec = datetime.sec;
  ts.tm_min = datetime.min;
  ts.tm_hour = datetime.hour;
  ts.tm_mday = datetime.day;
  ts.tm_mon = datetime.month - 1;
  ts.tm_year = datetime.year - 1900;
  time_t tsec = mktime(&ts);
  return (uint64_t)(tsec * 1000);
}
