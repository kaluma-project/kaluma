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
  // int res = km_storage_set_item(key, value);
  int ret = storage_set_item(key, value);
  if (ret < 0) {
    return create_system_error(ret);
  }
  return jerry_create_undefined();
}

/**
 * exports.getItem function
 */
JERRYXX_FUN(storage_get_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  int len = storage_get_item_value_length(key);
  char *buf = (char *)malloc(len + 1);
  int ret = storage_get_item(key, buf);
  if (ret < 0) {
    return create_system_error(ret);
  }
  jerry_value_t value = jerry_create_string((const jerry_char_t *)buf);
  free(buf);
  return value;
}

/**
 * exports.removeItem function
 */
JERRYXX_FUN(storage_remove_item_fn){
    JERRYXX_CHECK_ARG_STRING(0, "key") JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
    /*
    int res = km_storage_remove_item(key);
    if (res > -1) {
      return jerry_create_undefined();
    } else {  // failure
      return jerry_create_undefined();
    }
    */
}

/**
 * exports.clear function
 */
JERRYXX_FUN(storage_clear_fn) {
  int ret = storage_clear();
  if (ret < 0) {
    return create_system_error(ret);
  }
  return jerry_create_undefined();
}

/**
 * exports.length function
 */
JERRYXX_FUN(storage_length_fn) {
  int ret = storage_get_item_count();
  if (ret < 0) {
    return create_system_error(ret);
  }
  return jerry_create_number(ret);
}

/**
 * exports.key function
 */
JERRYXX_FUN(storage_key_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "index")
  int index = (int)JERRYXX_GET_ARG_NUMBER(0);
  /*
  char *buf = (char *)malloc(256);
  int res = km_storage_key(index, buf);
  if (res >= KM_STORAGE_OK) {
    jerry_value_t ret = jerry_create_string((const jerry_char_t *)buf);
    free(buf);
    return ret;
  } else {  // key not found
    free(buf);
    return jerry_create_null();
  }
  */
}

/**
 * Initialize 'storage' module and return exports
 */
jerry_value_t module_storage_init() {
  /* storage module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_STORAGE_SET_ITEM,
                                storage_set_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_GET_ITEM,
                                storage_get_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_REMOVE_ITEM,
                                storage_remove_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_CLEAR, storage_clear_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_LENGTH,
                                storage_length_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_KEY, storage_key_fn);
  return exports;
}
