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

#include <stdio.h>
#include "jerryscript.h"

#define JERRYXX_FUN(name) static jerry_value_t name(const jerry_value_t func_value, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt)

#define JERRYXX_CHECK_ARG(index, argname) \
  if (args_cnt <= index) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument required", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_ARG_NUMBER(index, argname) \
  if ((args_cnt <= index) || (!jerry_value_is_number(args_p[index]))) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument must be a number", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_ARG_NUMBER_OPT(index, argname) \
  if (args_cnt > index) { \
    if (!jerry_value_is_number(args_p[index])) { \
      char errmsg[255]; \
      sprintf(errmsg, "\"%s\" argument must be a number", argname); \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
    } \
  }

#define JERRYXX_CHECK_ARG_BOOLEAN_OPT(index, argname) \
  if (args_cnt > index) { \
    if (!jerry_value_is_boolean(args_p[index])) { \
      char errmsg[255]; \
      sprintf(errmsg, "\"%s\" argument must be a boolean", argname); \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
    } \
  }

#define JERRYXX_CHECK_ARG_FUNCTION(index, argname) \
  if ((args_cnt <= index) || (!jerry_value_is_function(args_p[index]))) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument must be a function", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_ARG_STRING(index, argname) \
  if ((args_cnt <= index) || (!jerry_value_is_string(args_p[index]))) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument must be a string", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_ARG_STRING_OPT(index, argname) \
  if (args_cnt > index) { \
    if (!jerry_value_is_string(args_p[index])) { \
      char errmsg[255]; \
      sprintf(errmsg, "\"%s\" argument must be a string", argname); \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
    } \
  }

#define JERRYXX_CHECK_ARG_OBJECT(index, argname) \
  if ((args_cnt <= index) || (!jerry_value_is_object(args_p[index]))) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument must be an object", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_ARG_OBJECT_OPT(index, argname) \
  if (args_cnt > index) { \
    if (!jerry_value_is_object(args_p[index])) { \
      char errmsg[255]; \
      sprintf(errmsg, "\"%s\" argument must be an object", argname); \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
    } \
  }

#define JERRYXX_CHECK_ARG_OBJECT_NULL_OPT(index, argname) \
  if (args_cnt > index) { \
    if (!(jerry_value_is_object(args_p[index]) || jerry_value_is_null(args_p[index]))) { \
      char errmsg[255]; \
      sprintf(errmsg, "\"%s\" argument must be an object or null", argname); \
      return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
    } \
  }

#define JERRYXX_CHECK_ARG_ARRAYBUFFER(index, argname) \
  if ((args_cnt <= index) || (!jerry_value_is_arraybuffer(args_p[index]))) { \
    char errmsg[255]; \
    sprintf(errmsg, "\"%s\" argument must be an ArrayBuffer", argname); \
    return jerry_create_error(JERRY_ERROR_TYPE, (const jerry_char_t *) errmsg); \
  }

#define JERRYXX_CHECK_INDEX_RANGE(name, lowerbound, upperbound) \
  if (name < (lowerbound) || name > (upperbound)) { \
    return jerry_create_error(JERRY_ERROR_RANGE, (const jerry_char_t *) "Index out of range"); \
  }

#define JERRYXX_GET_THIS this_val
#define JERRYXX_HAS_ARG(index) (args_cnt > index)
#define JERRYXX_GET_ARG_COUNT args_cnt
#define JERRYXX_GET_ARG(index) args_p[index]
#define JERRYXX_GET_ARG_NUMBER(index) jerry_get_number_value(args_p[index])
#define JERRYXX_GET_ARG_NUMBER_OPT(index, default) (args_cnt > index ? jerry_get_number_value(args_p[index]) : default)
#define JERRYXX_GET_ARG_BOOLEAN_OPT(index, default) (args_cnt > index ? jerry_get_boolean_value(args_p[index]) : default)
#define JERRYXX_GET_ARG_STRING_AS_CHAR(index, name) \
  jerry_size_t name##_sz = jerry_get_string_size(args_p[index]); \
  char name[name##_sz + 1]; \
  jerry_string_to_char_buffer(args_p[index], (jerry_char_t *)name, name##_sz); \
  name[name##_sz] = '\0';

#define JERRYXX_CREATE_ERROR(errmsg) jerry_create_error(JERRY_ERROR_COMMON, (const jerry_char_t *) errmsg)

void jerryxx_set_property(jerry_value_t object, const char *name, jerry_value_t value);
void jerryxx_set_property_number(jerry_value_t object, const char *name, double value);
void jerryxx_set_property_string(jerry_value_t object, const char *name, char *value);
void jerryxx_set_property_function(jerry_value_t object, const char *name, jerry_external_handler_t fn);
jerry_value_t jerryxx_get_property(jerry_value_t object, const char *name);
double jerryxx_get_property_number(jerry_value_t object, const char *name, double default_value);
bool jerryxx_delete_property(jerry_value_t object, const char *name);

void jerryxx_print_value(jerry_value_t value);
void jerryxx_print_error(jerry_value_t value, bool print_stacktrace);

jerry_size_t jerryxx_get_ascii_string_size(const jerry_value_t value);
jerry_size_t jerryxx_get_ascii_string_length(const jerry_value_t value);
jerry_size_t jerryxx_string_to_ascii_char_buffer(const jerry_value_t value, jerry_char_t *buf, jerry_size_t len);

#endif /* __JERRYXX_H */