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
#include "jerryxx.h"
#include "tty.h"
#include "repl.h"

static jerry_value_t console_log(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == *);
  if (args_cnt > 0) {
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[0m"); // set to normal color
    for (int i = 0; i < args_cnt; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value("%s", args_p[i]);
    }
    repl_printf("\r\n");
    repl_print_end();
  }
  return jerry_create_undefined();
}

static jerry_value_t console_error(const jerry_value_t func_value,
  const jerry_value_t this_val, const jerry_value_t args_p[],
  const jerry_length_t args_cnt) {
  // ASSERT(args_cnt == *);
  if (args_cnt > 0) {
    repl_print_begin(REPL_OUTPUT_ERROR);
    repl_printf("\33[2K\r"); // set column to 0
    repl_printf("\33[31m"); // red
    for (int i = 0; i < args_cnt; i++) {
      if (i > 0) {
        repl_printf(" ");
      }
      repl_print_value("%s", args_p[i]);
    }
    repl_printf("\r\n");
    repl_printf("\33[0m"); // back to normal color
    repl_print_end();
  }
  return jerry_create_undefined();
}

jerry_value_t module_console_init() {
  jerry_value_t object = jerry_create_object();
  jerryxx_set_propery_function(object, "log", console_log);
  jerryxx_set_propery_function(object, "error", console_error);
  return object;
}
