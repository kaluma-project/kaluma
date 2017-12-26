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

#ifndef __JERRYXX_H
#define __JERRYXX_H

#include "jerryscript.h"

#define JSX_FUN(name) static jerry_value_t name(const jerry_value_t func_value, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt)

#define JSX_CHECK_ARG_NUMBER(index) \
  if ((args_cnt <= index) || (!jerry_value_is_number(args_p[index]))) { \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Bad arguments"); \
  }

#define JSX_CHECK_ARG_NUMBER_OPT(index) \
  if (args_cnt > index) { \
    if (!jerry_value_is_number(args_p[index])) { \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Bad arguments"); \
    } \
  }

#define JSX_CHECK_ARG_BOOLEAN_OPT(index) \
  if (args_cnt > index) { \
    if (!jerry_value_is_boolean(args_p[index])) { \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) "Bad arguments"); \
    } \
  }

#define JSX_GET_ARG_NUMBER(index) jerry_get_number_value(args_p[index])
#define JSX_GET_ARG_NUMBER_OPT(index, default) (args_cnt > index ? jerry_get_number_value(args_p[index]) : default)
#define JSX_GET_ARG_BOOLEAN_OPT(index, default) (args_cnt > index ? jerry_get_boolean_value(args_p[index]) : default)

void jerryxx_set_propery_number(jerry_value_t object, const char *name, double value);
void jerryxx_set_propery_object(jerry_value_t object, const char *name, jerry_value_t obj);
void jerryxx_set_propery_function(jerry_value_t object, const char *name, jerry_external_handler_t fn);

#endif /* __JERRYXX_H */