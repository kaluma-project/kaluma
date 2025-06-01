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

#include "err.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "wdt.h"
#include "wdt_magic_strings.h"

JERRYXX_FUN(wdt_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "timeout");
  uint32_t timeout_ms = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_WDT_TIMEOUT, timeout_ms);
  int ret = km_wdt_enable(true, timeout_ms);
  if (ret < 0) {
    return jerry_error_sz(JERRY_ERROR_REFERENCE,
                              "This board does not support Watch dog timer.");
  } else {
    return jerry_undefined();
  }
}

JERRYXX_FUN(wdt_feed_fn) {
  km_wdt_feed();
  return jerry_undefined();
}

jerry_value_t module_wdt_init() {
  /* WDT class */
  jerry_value_t wdt_ctor = jerry_function_external(wdt_ctor_fn);
  jerry_value_t prototype = jerry_object();
  jerryxx_set_property(wdt_ctor, "prototype", prototype);
  jerryxx_set_property_function(prototype, MSTR_WDT_FEED, wdt_feed_fn);
  jerry_value_free(prototype);

  /* wdt module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_WDT_WDT, wdt_ctor);
  jerry_value_free(wdt_ctor);

  return exports;
}
