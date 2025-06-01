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

#include <stdlib.h>

#include "jerryscript.h"
#include "jerryxx.h"
#include "rtc.h"
#include "rtc_magic_strings.h"

/**
 * rtc.setTime(t)
 * args:
 *   time: {number}
 */
JERRYXX_FUN(rtc_set_time_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "time")
  double time = JERRYXX_GET_ARG_NUMBER(0);
  km_rtc_set_time((uint64_t)time);
  return jerry_undefined();
}

/**
 * rtc.getTime(t)
 * returns:
 *   {number}
 */
JERRYXX_FUN(rtc_get_time_fn) {
  uint64_t time = km_rtc_get_time();
  return jerry_number((double)time);
}

/**
 * Initialize 'rtc' module
 */
jerry_value_t module_rtc_init() {
  /* rtc module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property_function(exports, MSTR_RTC_SET_TIME, rtc_set_time_fn);
  jerryxx_set_property_function(exports, MSTR_RTC_GET_TIME, rtc_get_time_fn);
  return exports;
}
