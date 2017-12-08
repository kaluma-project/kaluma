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

#include <stdlib.h>
#include "jerryscript.h"
#include "io.h"
#include "runtime.h"

static void set_timer_cb(io_timer_handle_t *timer) {
  if (jerry_value_is_function(timer->timer_js_cb)) {
    jerry_value_t this_val = jerry_create_undefined ();
    jerry_value_t ret_val = jerry_call_function (timer->timer_js_cb, this_val, NULL, 0);
    if (!jerry_value_has_error_flag (ret_val)) {
      // handle return value
    }
    jerry_release_value (ret_val);
    jerry_release_value (this_val);
  } 
}

static jerry_value_t set_timer(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 3);
  // ASSERT(jerry_value_is_function(args_p[0]))
  // ASSERT(jerry_value_is_number(args_p[1]))
  // ASSERT(jerry_value_is_boolean(args_p[2]))
  io_timer_handle_t *timer = malloc(sizeof(io_timer_handle_t));
  io_timer_init(timer);
  timer->timer_js_cb = args_p[0];
  uint64_t interval = (uint64_t) jerry_get_number_value(args_p[1]);
  bool repeat = jerry_get_boolean_value(args_p[2]);
  io_timer_start(timer, set_timer_cb, interval, repeat);
  return jerry_create_number(timer->timer_id);
}

static void timer_close_cb(io_handle_t *handle) {
  free(handle);
}

static jerry_value_t clear_timer(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == 1);
  // ASSERT(jerry_value_is_number(args_p[0]))
  int timer_id = (int) jerry_get_number_value(args_p[0]);
  io_timer_handle_t *timer = io_timer_get_by_id(timer_id);
  io_timer_stop(timer);
  io_handle_close((io_handle_t *) timer, timer_close_cb);
  return jerry_create_undefined();
}

jerry_value_t module_timers_init() {
  jerry_value_t object = jerry_create_object();
  runtime_register_function(object, "setTimer", set_timer);
  runtime_register_function(object, "clearTimer", clear_timer);
  return object;
}
