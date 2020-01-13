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
#include "storage_magic_strings.h"
#include "storage.h"

/**
 * exports.setItem function
 */
JERRYXX_FUN(storage_set_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_CHECK_ARG_STRING(1, "value")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, value)
  int res = storage_set_item(key, value);
  return jerry_create_number(res);
}

/**
 * exports.getItem function
 */
JERRYXX_FUN(storage_get_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  char buf[256];
  int res = storage_get_item(key, buf);
  if (res > -1) {
    return jerry_create_string(buf);
  } else { // key not found
    return jerry_create_null();
  }
}

/**
 * exports.removeItem function
 */
JERRYXX_FUN(storage_remove_item_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "key")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, key)
  int res = storage_remove_item(key);
  if (res > -1) {
    return jerry_create_undefined();
  } else { // failure
    return jerry_create_undefined();
  }
}

/**
 * exports.clear function
 */
JERRYXX_FUN(storage_clear_fn) {
  int res = storage_clear();
  if (res > -1) {
    return jerry_create_undefined();
  } else { // failure
    return jerry_create_undefined();
  }
}

/**
 * exports.length function
 */
JERRYXX_FUN(storage_length_fn) {
  int len = storage_length();
  if (len > -1) {
    return jerry_create_number(len);
  } else { // failure
    return jerry_create_undefined();
  }
}

/**
 * exports.key function
 */
JERRYXX_FUN(storage_key_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "index")
  int index = (int) JERRYXX_GET_ARG_NUMBER(0);
  char buf[256];
  int res = storage_key(index, buf);
  if (res > -1) {
    return jerry_create_string(buf);
  } else { // failure
    return jerry_create_undefined();
  }
}

/**
 * Initialize 'storage' module and return exports
 */
jerry_value_t module_storage_init() {
  /* storage module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_STORAGE_SET_ITEM, storage_set_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_GET_ITEM, storage_get_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_REMOVE_ITEM, storage_remove_item_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_CLEAR, storage_clear_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_LENGTH, storage_length_fn);
  jerryxx_set_property_function(exports, MSTR_STORAGE_KEY, storage_key_fn);
  return exports;
}
