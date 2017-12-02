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

#include "jerryscript.h"
#include "global.h"

static jerry_value_t
set_timer(const jerry_value_t func_value, /**< function object */
          const jerry_value_t this_val, /**< this arg */
          const jerry_value_t args_p[], /**< function arguments */
          const jerry_length_t args_cnt) /**< number of function arguments */
{
  return jerry_create_string((const jerry_char_t *) "native set_timer() called...");
}

jerry_value_t module_timers_init() {
  jerry_value_t object = jerry_create_object();

  /* Add `process.object.set_timer` property */
  jerry_value_t set_timer_fn = jerry_create_external_function(set_timer);
  jerry_value_t set_timer_prop = jerry_create_string((const jerry_char_t *) "set_timer");
  jerry_set_property (object, set_timer_prop, set_timer_fn);
  jerry_release_value (set_timer_prop);
  jerry_release_value(set_timer_fn);

  return object;
}
