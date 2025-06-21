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
#include "storage.h"
#include "storage_magic_strings.h"

/**
 * exports.setItem function
 */
JERRYXX_FUN(storage_set_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_CHECK_ARG_STRING(1, "value")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, value)
  int ret = storage_set_item(key, value);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * exports.getItem function
 */
JERRYXX_FUN(storage_get_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  int len = storage_get_value_length(key);
  if (len < 0) {
    return jerry_null();
  }
  char *buf = (char *)malloc(len);
  storage_get_value(key, buf);
  jerry_value_t value = jerry_string((const jerry_char_t *)buf, len, JERRY_ENCODING_CESU8);
  free(buf);
  return value;
}

/**
 * exports.removeItem function
 */
JERRYXX_FUN(storage_remove_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  storage_remove_item(key);
  return jerry_undefined();
}

/**
 * exports.clear function
 */
JERRYXX_FUN(storage_clear_fn) {
  int ret = storage_clear();
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * exports.length function
 */
JERRYXX_FUN(storage_length_fn) {
  int ret = storage_get_item_count();
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_number(ret);
}

/**
 * exports.key function
 */
JERRYXX_FUN(storage_key_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "index")
  int index = (int)JERRYXX_GET_ARG_NUMBER(0);
  int len = storage_get_key_length(index);
  if (len < 0) {
    return jerry_null();
  }
  char *buf = (char *)malloc(len);
  storage_get_key(index, buf);
  jerry_value_t ret = jerry_string((const jerry_char_t *)buf, len, JERRY_ENCODING_CESU8);
  free(buf);
  return ret;
}

/**
 * Initialize 'storage' module and return exports
 */
jerry_value_t module_storage_init() {
  /* storage module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property_function(exports, MSTR_STORAGE_SET_ITEM,
                                storage_set_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_GET_ITEM,
                                storage_get_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_REMOVE_ITEM,
                                storage_remove_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_CLEAR, storage_clear_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_KEY, storage_key_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_LENGTH,
                                storage_length_fn);
  // jerryxx_define_own_property(exports, MSTR_STORAGE_LENGTH,
  // storage_length_fn, NULL);
  return exports;
}
