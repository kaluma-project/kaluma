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

void runtime_init() {
  jerry_init (JERRY_INIT_EMPTY);
}


void runtime_test() {
  const jerry_char_t script[] = "print ('Hello, World!');";
  size_t script_size = strlen ((const char *) script);
  jerryx_handler_register_global ((const jerry_char_t *) "print", jerryx_handler_print);
  jerry_value_t parsed_code = jerry_parse (script, script_size, false);
  if (!jerry_value_has_error_flag (parsed_code)) {
    jerry_value_t ret_value = jerry_run (parsed_code);
    jerry_release_value (ret_value);
  }
  jerry_release_value (parsed_code);
}

void runtime_deinit() {
  jerry_cleanup ();  
}