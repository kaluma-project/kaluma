/* Copyright (c) 2017 Pico
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
#include "native_magic_strings.h"

JERRYXX_FUN(native_addr_fn) {
  JERRYXX_CHECK_ARG(0, "value");
  jerry_value_t obj = JERRYXX_GET_ARG(0);
  return jerry_create_number((double)obj);
}

JERRYXX_FUN(native_obj_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "address");
  return (jerry_value_t)(uint32_t)JERRYXX_GET_ARG_NUMBER(0);
}

JERRYXX_FUN(native_rb_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "address");
  uint8_t *addr = (uint8_t*)(uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  return jerry_create_number((double)*addr);
}

JERRYXX_FUN(native_rw_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "address");
  uint32_t *addr = (uint32_t*)(uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  return jerry_create_number((double)*addr);
}

JERRYXX_FUN(native_hb_fn) {
  char str[20];
  JERRYXX_CHECK_ARG_NUMBER(0, "number");
  uint32_t num = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  sprintf(str,"%02X",(unsigned int)num);
  return jerry_create_string((unsigned char *)str);
}

JERRYXX_FUN(native_hw_fn) {
  char str[20];
  JERRYXX_CHECK_ARG_NUMBER(0, "number");
  uint32_t num = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  sprintf(str,"%08X",(unsigned int)num);
  return jerry_create_string((unsigned char *)str);
}

JERRYXX_FUN(native_ro_fn) {
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint32_t offset = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  jerry_length_t byteLength = 0;
  jerry_length_t byteOffset = 0;
  jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
  uint16_t *buf = (uint16_t*)jerry_get_arraybuffer_pointer(array_buffer);
  return jerry_create_number(buf[offset]);
}

JERRYXX_FUN(native_rfill_fn) {
  jerry_value_t data = JERRYXX_GET_ARG(0);
  uint16_t val = (uint16_t)JERRYXX_GET_ARG_NUMBER(1);
  uint32_t len = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  jerry_length_t byteLength = 0;
  jerry_length_t byteOffset = 0;
  jerry_value_t array_buffer = jerry_get_typedarray_buffer(data, &byteOffset, &byteLength);
  uint16_t *buf = (uint16_t*)jerry_get_arraybuffer_pointer(array_buffer);
  for(int i=0;i<len;i++) buf[i]=val;
  return jerry_create_undefined();
}

JERRYXX_FUN(native_sprdraw_fn) {
  jerry_value_t src_data = JERRYXX_GET_ARG(0);
  jerry_length_t byteLength = 0;
  jerry_length_t byteOffset = 0;
  jerry_value_t src_buffer = jerry_get_typedarray_buffer(src_data, &byteOffset, &byteLength);
  uint16_t *src = (uint16_t*)jerry_get_arraybuffer_pointer(src_buffer);
  uint32_t xlen = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  uint32_t ylen = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  jerry_value_t dst_data = JERRYXX_GET_ARG(3);
  jerry_value_t dst_buffer = jerry_get_typedarray_buffer(dst_data, &byteOffset, &byteLength);
  uint16_t *dst = (uint16_t*)jerry_get_arraybuffer_pointer(dst_buffer);
  uint32_t xdst = (uint32_t)JERRYXX_GET_ARG_NUMBER(4);
  uint32_t ydst = (uint32_t)JERRYXX_GET_ARG_NUMBER(5);
  for(int y=0;y<ylen;y++){
    uint32_t yso = y*xlen;
    uint32_t ydo = ((ydst+y)%160)*128;
    for(int x=0;x<xlen;x++){
      uint32_t xdo = (xdst+x)%128;
      if(src[yso+x]!=0) dst[ydo+xdo]=src[yso+x];
    }
  }
  jerry_release_value(src_buffer);
  jerry_release_value(dst_buffer);
  return jerry_create_undefined();
}

jerry_value_t module_native_init() {
  /* rtc module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_NATIVE_ADDR, native_addr_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_OBJ, native_obj_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_RB, native_rb_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_RW, native_rw_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_HB, native_hb_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_HW, native_hw_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_SPRDRAW, native_sprdraw_fn);

  jerryxx_set_property_function(exports, MSTR_NATIVE_RO, native_ro_fn);
  jerryxx_set_property_function(exports, MSTR_NATIVE_RFILL, native_rfill_fn);
  return exports;
}
