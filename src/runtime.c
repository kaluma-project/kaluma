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

#include <string.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

#include "repl.h"
#include "kameleon_js.h"

/**
 * Get a string from a native object
 */
static jerry_value_t
get_msg_handler (const jerry_value_t func_value, /**< function object */
                 const jerry_value_t this_value, /**< this arg */
                 const jerry_value_t *args_p, /**< function arguments */
                 const jerry_length_t args_cnt) /**< number of function arguments */
{
  return jerry_create_string ((const jerry_char_t *) "return string test.");
} /* get_msg_handler */

static void runtime_global_objects() {
  jerryx_handler_register_global ((const jerry_char_t *) "print", jerryx_handler_print);

  /* process module */
  jerry_value_t object = jerry_create_object();

  jerry_value_t func_obj = jerry_create_external_function (get_msg_handler);

  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "myFunc");
  jerry_set_property (object, prop_name, func_obj);
  jerry_release_value (prop_name);
  jerry_release_value (func_obj);

  jerry_value_t global_object = jerry_get_global_object ();

  prop_name = jerry_create_string ((const jerry_char_t *) "process");
  jerry_set_property (global_object, prop_name, object);
  jerry_release_value (prop_name);
  jerry_release_value (object);
  jerry_release_value (global_object);
}

void runtime_init() {
  jerry_init (JERRY_INIT_EMPTY);
  runtime_global_objects();
}

void runtime_test() {
  jerry_value_t res = jerry_exec_snapshot (startup_s, startup_l, true);

  jerry_value_t this_val = jerry_create_undefined ();
  jerry_value_t ret_val = jerry_call_function (res, this_val, NULL, 0);

  print_value(res);
  print_value(ret_val);

  jerry_release_value (ret_val);
  jerry_release_value (this_val);  
  jerry_release_value (res);
}

void runtime_deinit() {
  jerry_cleanup ();  
}