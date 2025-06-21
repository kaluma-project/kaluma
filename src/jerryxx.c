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

#include "jerryxx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jerryscript.h"
#include "magic_strings.h"
#include "repl.h"
#include "tty.h"

void jerryxx_set_property(jerry_value_t object, const char *name,
                          jerry_value_t value) {
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_set(object, prop, value);
  jerry_value_free(ret);
  jerry_value_free(prop);
}

void jerryxx_set_property_number(jerry_value_t object, const char *name,
                                 double value) {
  jerry_value_t val = jerry_number(value);
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_set(object, prop, val);
  jerry_value_free(ret);
  jerry_value_free(prop);
  jerry_value_free(val);
}

void jerryxx_set_property_string(jerry_value_t object, const char *name,
                                 char *value) {
  jerry_value_t val = jerry_string_sz((const char *)value);
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_set(object, prop, val);
  jerry_value_free(ret);
  jerry_value_free(prop);
  jerry_value_free(val);
}

void jerryxx_set_property_function(jerry_value_t object, const char *name,
                                   jerry_external_handler_t fn) {
  jerry_value_t ext_fn = jerry_function_external(fn);
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_set(object, prop, ext_fn);
  jerry_value_free(ret);
  jerry_value_free(prop);
  jerry_value_free(ext_fn);
}

void jerryxx_define_own_property(jerry_value_t object, const char *name,
                                 jerry_external_handler_t getter,
                                 jerry_external_handler_t setter) {
  // storage.length readonly property
  jerry_property_descriptor_t prop = jerry_property_descriptor();
  prop.flags &= ~JERRY_PROP_IS_WRITABLE;
  if (getter != NULL) {
    prop.flags |= JERRY_PROP_IS_GET_DEFINED;
    prop.getter = jerry_function_external(getter);
  }
  if (setter != NULL) {
    prop.setter = jerry_function_external(setter);
    prop.flags |= (JERRY_PROP_IS_WRITABLE_DEFINED | JERRY_PROP_IS_WRITABLE);
  }
  jerry_value_t prop_name = jerry_string_sz((const char *)name);
  jerry_object_define_own_prop(object, prop_name, &prop);
  jerry_value_free(prop_name);
  jerry_property_descriptor_free(&prop);
}

jerry_value_t jerryxx_get_property(jerry_value_t object, const char *name) {
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_get(object, prop);
  jerry_value_free(prop);
  return ret;
}

double jerryxx_get_property_number(jerry_value_t object, const char *name,
                                   double default_value) {
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_get(object, prop);
  double value = default_value;
  if (jerry_value_is_number(ret)) {
    value = jerry_value_as_number(ret);
  }
  jerry_value_free(ret);
  jerry_value_free(prop);
  return value;
}

bool jerryxx_get_property_boolean(jerry_value_t object, const char *name,
                                  bool default_value) {
  jerry_value_t prop = jerry_string_sz((const char *)name);
  jerry_value_t ret = jerry_object_get(object, prop);
  bool value = default_value;
  if (jerry_value_is_boolean(ret)) {
    value = jerry_value_is_true(ret);
  }
  jerry_value_free(ret);
  jerry_value_free(prop);
  return value;
}

uint8_t *jerryxx_get_typedarray_buffer(jerry_value_t object) {
  jerry_length_t length = 0;
  jerry_length_t offset = 0;
  jerry_value_t arrbuf = jerry_typedarray_buffer(object, &offset, &length);
  uint8_t *buffer_pointer = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  return buffer_pointer;
}

bool jerryxx_delete_property(jerry_value_t object, const char *name) {
  jerry_value_t prop = jerry_string_sz((const char *)name);
  bool ret = jerry_object_delete(object, prop);
  jerry_value_free(prop);
  return ret;
}

void jerryxx_array_push_string(jerry_value_t array, jerry_value_t item) {
  jerry_value_t push = jerryxx_get_property(array, "push");
  jerry_value_t _args[] = {item};
  jerry_call(push, array, _args, 1);
  jerry_value_free(push);
}

void jerryxx_print_value(jerry_value_t value) {
  jerry_value_t str = jerry_value_to_string(value);
  jerry_size_t str_sz = jerry_string_size(str, JERRY_ENCODING_CESU8);
  jerry_char_t str_buf[str_sz + 1];
  jerry_string_to_buffer(str, JERRY_ENCODING_CESU8, str_buf, str_sz);
  for (int16_t i = 0; i < str_sz; i++) km_tty_putc(str_buf[i]);
  jerry_value_free(str);
}

/**
 * Print error with stacktrace
 */
void jerryxx_print_error(jerry_value_t value, bool print_stacktrace) {
  jerry_value_t error_value = jerry_throw(value, false);
  // print error message
  jerry_value_t err_str = jerry_value_to_string(error_value);
  km_repl_set_output(KM_REPL_OUTPUT_ERROR);
  km_repl_print_value(err_str);
  km_repl_println();
  km_repl_set_output(KM_REPL_OUTPUT_NORMAL);
  jerry_value_free(err_str);
  // print stack trace
  if (print_stacktrace && jerry_value_is_object(error_value)) {
    jerry_value_t stack_str =
        jerry_string_sz((const char *)"stack");
    jerry_value_t backtrace_val = jerry_object_get(error_value, stack_str);
    jerry_value_free(stack_str);
    if (!jerry_value_is_error(backtrace_val) &&
        jerry_value_is_array(backtrace_val)) {
      uint32_t length = jerry_array_length(backtrace_val);
      if (length > 32) {
        length = 32;
      } /* max length: 32 */
      for (uint32_t i = 0; i < length; i++) {
        jerry_value_t item_val = jerry_object_get_index(backtrace_val, i);
        if (!jerry_value_is_error(item_val) &&
            jerry_value_is_string(item_val)) {
          km_repl_set_output(KM_REPL_OUTPUT_ERROR);
          km_repl_printf("  at ");
          km_repl_print_value(item_val);
          km_repl_println();
          km_repl_set_output(KM_REPL_OUTPUT_NORMAL);
        }
        jerry_value_free(item_val);
      }
    }
    jerry_value_free(backtrace_val);
  }
  jerry_value_free(error_value);
}

jerry_size_t jerryxx_get_ascii_string_size(const jerry_value_t value) {
  return jerry_string_length(value);
}

jerry_size_t jerryxx_get_ascii_string_length(const jerry_value_t value) {
  return jerry_string_length(value);
}

jerry_size_t jerryxx_string_to_ascii_char_buffer(const jerry_value_t value,
                                                 jerry_char_t *buf,
                                                 jerry_size_t len) {
  jerry_size_t utf8_sz = jerry_string_size(value, JERRY_ENCODING_UTF8);
  jerry_char_t utf8_buf[utf8_sz];
  jerry_string_to_buffer(value, JERRY_ENCODING_UTF8, utf8_buf, utf8_sz);
  uint32_t utf8_p = 0;
  uint32_t ascii_p = 0;
  while (ascii_p < len) {
    uint8_t ch = utf8_buf[utf8_p];
    if (ch < 128) {  // 1 byte
      buf[ascii_p] = ch;
      utf8_p++;
    } else if (ch >> 5 == 6) {  // 2 bytes
      if (ch == 0xc2) {
        buf[ascii_p] = utf8_buf[utf8_p + 1];
      } else if (ch == 0xc3) {
        buf[ascii_p] = utf8_buf[utf8_p + 1] + 64;
      }
      utf8_p += 2;
    } else if (ch >> 4 == 14) {  // 3 bytes
      buf[ascii_p] = 0;          // Don't encode over 2 bytes
      utf8_p += 3;
    } else if (ch >> 3 == 30) {  // 4 bytes
      buf[ascii_p] = 0;          // Don't encode over 2 bytes
      utf8_p += 4;
    }
    ascii_p++;
  }
  return ascii_p;
}

jerry_value_t jerryxx_call_require(const char *name) {
  jerry_value_t global_js = jerry_current_realm();
  jerry_value_t require_js = jerryxx_get_property(global_js, MSTR_REQUIRE);
  jerry_value_t this_js = jerry_undefined();
  jerry_value_t name_js = jerry_string_sz((const char *)name);
  jerry_value_t args_js[1] = {name_js};
  jerry_value_t ret = jerry_call(require_js, this_js, args_js, 1);
  jerry_value_free(name_js);
  jerry_value_free(this_js);
  jerry_value_free(require_js);
  jerry_value_free(global_js);
  return ret;
}

jerry_value_t jerryxx_call_method(jerry_value_t obj, char *name,
                                  jerry_value_t *args, int args_count) {
  jerry_value_t method = jerryxx_get_property(obj, name);
  jerry_value_t ret = jerry_call(method, obj, args, args_count);
  jerry_value_free(method);
  return ret;
}

void jerryxx_inherit(jerry_value_t super_ctor, jerry_value_t sub_ctor) {
  // Subclass.prototype = Object.create(Superclass.prototype);
  jerry_value_t global = jerry_current_realm();
  jerry_value_t global_object = jerryxx_get_property(global, MSTR_OBJECT);
  jerry_value_t global_object_create =
      jerryxx_get_property(global_object, MSTR_CREATE);
  jerry_value_t super_ctor_prototype =
      jerryxx_get_property(super_ctor, MSTR_PROTOTYPE);
  jerry_value_t _args[1] = {super_ctor_prototype};
  jerry_value_t sub_ctor_prototype =
      jerry_call(global_object_create, global_object, _args, 1);
  jerryxx_set_property(sub_ctor, MSTR_PROTOTYPE, sub_ctor_prototype);
  jerry_value_free(sub_ctor_prototype);
  jerry_value_free(super_ctor_prototype);
  jerry_value_free(global_object_create);
  jerry_value_free(global_object);
  jerry_value_free(global);

  // Subclass.prototype.constructor = Subclass
  jerryxx_set_property(sub_ctor_prototype, MSTR_CONSTRUCTOR, sub_ctor);
}
