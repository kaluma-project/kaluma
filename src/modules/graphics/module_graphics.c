/* Copyright (c) 2019 Kameleon
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
#include "graphics_magic_strings.h"
#include "module_graphics.h"

/**
 * MonoGraphicContext() constructor
 */
JERRYXX_FUN(mono_graphic_context_ctor_fn) {
  JERRYXX_CHECK_ARG_OBJECT(0, "driver");
  jerry_value_t driver = JERRYXX_GET_ARG(0);
  uint16_t width = (uint16_t) jerryxx_get_property_number(driver, MSTR_GRAPHICS_WIDTH, 0);
  uint16_t height = (uint16_t) jerryxx_get_property_number(driver, MSTR_GRAPHICS_HEIGHT, 0);
  uint16_t sz = width * ((height + 7) / 8);
  jerry_value_t buffer = jerry_create_arraybuffer(sz);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_DRIVER, driver);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_GRAPHICS_WIDTH, width);
  jerryxx_set_property_number(JERRYXX_GET_THIS, MSTR_GRAPHICS_HEIGHT, height);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER, buffer);
  return jerry_create_undefined();
}


/**
 * MonoGraphicContext.prototype.setPixel(x, y, color) function
 */
JERRYXX_FUN(mono_graphic_context_set_pixel_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "color")
  uint16_t x = (uint16_t) JERRYXX_GET_ARG_NUMBER(0);
  uint16_t y = (uint16_t) JERRYXX_GET_ARG_NUMBER(1);
  uint16_t color = (uint16_t) JERRYXX_GET_ARG_NUMBER(2);
  uint16_t width = (uint16_t) jerryxx_get_property_number(JERRYXX_GET_THIS, MSTR_GRAPHICS_WIDTH, 0);
  //uint16_t height = (uint16_t) jerryxx_get_property_number(JERRYXX_GET_THIS, MSTR_GRAPHICS_HEIGHT, 0); //Never used.
  jerry_value_t buffer = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER);

  uint8_t *buf = jerry_get_arraybuffer_pointer (buffer);
  uint16_t idx = x + (y / 8) * width;
  uint8_t val = (1 << (y & 7));
  if (color) {
    buf[idx] |= val;
  } else {
    buf[idx] &= ~val;
  }

  // buf[idx] = buf[idx] | val;

  /*
  if (color) {
    buf[x + (y / 8) * width] |=  (1 << (y & 7));
  } else {
    buf[x + (y / 8) * width] &= ~(1 << (y & 7));
  }
  */
  jerry_release_value(buffer);
  return jerry_create_undefined();
}

/**
 * MonoGraphicContext.prototype.flush() function
 */
JERRYXX_FUN(mono_graphic_context_flush_fn) {
  jerry_value_t driver = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_DRIVER);
  jerry_value_t buffer = jerryxx_get_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER);

  jerry_value_t driver_flush = jerryxx_get_property(driver, MSTR_GRAPHICS_FLUSH);
  if (jerry_value_is_function (driver_flush)) {
    jerry_value_t args[] = { buffer };
    jerry_value_t ret_val = jerry_call_function (driver_flush, driver, args, 1);
    if (!jerry_value_is_error (ret_val)) {
      // TODO handle error
    }
    jerry_release_value(ret_val);
  }
  jerry_release_value(driver_flush);
  jerry_release_value(buffer);
  jerry_release_value(driver);
  return jerry_create_undefined();
}

/**
 * Initialize 'graphics' module
 */
jerry_value_t module_graphics_init() {
  /* MonoGraphicContext class */
  jerry_value_t mono_graphic_context_ctor = jerry_create_external_function(mono_graphic_context_ctor_fn);
  jerry_value_t mono_graphic_context_prototype = jerry_create_object();
  jerryxx_set_property(mono_graphic_context_ctor, "prototype", mono_graphic_context_prototype);
  jerryxx_set_property_function(mono_graphic_context_prototype, MSTR_GRAPHICS_SETPIXEL, mono_graphic_context_set_pixel_fn);
  jerryxx_set_property_function(mono_graphic_context_prototype, MSTR_GRAPHICS_FLUSH, mono_graphic_context_flush_fn);
  jerry_release_value (mono_graphic_context_prototype);

  /* graphics module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_GRAPHICS_MONO_GRAPHIC_CONTEXT, mono_graphic_context_ctor);
  jerry_release_value (mono_graphic_context_ctor);

  return exports;
}
