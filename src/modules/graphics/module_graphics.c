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
#include "gc.h"
#include "font.h"

gc_font_t custom_font;

static void gc_handle_freecb (void *handle) {
  free (handle);
}

static const jerry_object_native_info_t gc_handle_info = {
  .free_cb = gc_handle_freecb
};

#define JERRYXX_GET_NATIVE_HANDLE(name, handle_type, handle_info) \
  void *native_pointer; \
  bool has_p = jerry_get_object_native_pointer (this_val, &native_pointer, &handle_info); \
  if (!has_p) { \
    return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t *) "Failed to get native handle"); \
  } \
  handle_type *name = (handle_type *) native_pointer;


/**
 * GraphicContext() constructor
 */
JERRYXX_FUN(gc_ctor_fn) {
  JERRYXX_CHECK_ARG_OBJECT(0, "driver");

  // get driver
  jerry_value_t driver = JERRYXX_GET_ARG(0);
  int16_t device_width = (int16_t) jerryxx_get_property_number(driver, MSTR_GRAPHICS_WIDTH, 0);
  int16_t device_height = (int16_t) jerryxx_get_property_number(driver, MSTR_GRAPHICS_HEIGHT, 0);
  uint8_t rotation = (uint16_t) jerryxx_get_property_number(driver, MSTR_GRAPHICS_ROTATION, 0);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_DRIVER, driver);

  // allocate buffer
  uint16_t size = device_width * ((device_height + 7) / 8);
  jerry_value_t buffer = jerry_create_arraybuffer(size);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER, buffer);

  // set native handle
  gc_handle_t *gc_handle = (gc_handle_t *) malloc(sizeof (gc_handle_t));
  gc_handle->device_width = device_width;
  gc_handle->device_height = device_height;
  gc_set_rotation(gc_handle, rotation);
  gc_handle->buffer = jerry_get_arraybuffer_pointer(buffer);
  gc_handle->buffer_size = size;
  gc_handle->color = 1;
  gc_handle->fill_color = 1;
  gc_handle->font = NULL;
  gc_handle->font_color = 1;
  gc_handle->font_scale_x = 1;
  gc_handle->font_scale_y = 1;
  jerry_set_object_native_pointer(this_val, gc_handle, &gc_handle_info);
  
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getWidth()
 */
JERRYXX_FUN(gc_get_width_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  int16_t width = gc_get_width(gc_handle);
  return jerry_create_number(width);
}

/**
 * GraphicContext.prototype.getHeight()
 */
JERRYXX_FUN(gc_get_height_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  int16_t height = gc_get_height(gc_handle);
  return jerry_create_number(height);
}

/**
 * GraphicContext.prototype.clearScreen()
 */
JERRYXX_FUN(gc_clear_screen_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_clear_screen(gc_handle);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.fillScreen(color)
 */
JERRYXX_FUN(gc_fill_screen_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint8_t color = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_screen(gc_handle, color);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.setRotation(rotation)
 */
JERRYXX_FUN(gc_set_rotation_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "rotation")
  uint8_t rotation = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_rotation(gc_handle, rotation);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getRotation() -> rotation
 */
JERRYXX_FUN(gc_get_rotation_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint8_t rotation = gc_get_rotation(gc_handle);
  return jerry_create_number(rotation);
}

/**
 * GraphicContext.prototype.setColor(color)
 */
JERRYXX_FUN(gc_set_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint16_t color = (uint16_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_color(gc_handle, color);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getColor()
 */
JERRYXX_FUN(gc_get_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_color(gc_handle);
  return jerry_create_number(color);
}

/**
 * GraphicContext.prototype.setFillColor(color)
 */
JERRYXX_FUN(gc_set_fill_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint8_t color = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_fill_color(gc_handle, color);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getFillColor()
 */
JERRYXX_FUN(gc_get_fill_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_fill_color(gc_handle);
  return jerry_create_number(color);
}

/**
 * GraphicContext.prototype.setFontColor(color)
 */
JERRYXX_FUN(gc_set_font_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint8_t color = (uint8_t) JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_font_color(gc_handle, color);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getFontColor()
 */
JERRYXX_FUN(gc_get_font_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_font_color(gc_handle);
  return jerry_create_number(color);
}

/**
 * GraphicContext.prototype.setFont(font)
 */
JERRYXX_FUN(gc_set_font_fn) {
  // TODO: allow to accept "null" value for first argument
  JERRYXX_CHECK_ARG_OBJECT_OPT(0, "font")
  jerry_value_t font = JERRYXX_GET_ARG_OPT(0, 0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  if (jerry_value_is_object(font)) {
    custom_font.first = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_FIRST, 0);
    custom_font.last = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_LAST, 0);
    custom_font.width = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_WIDTH, 0);
    custom_font.height = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_HEIGHT, 0);
    custom_font.advance_x = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_ADVANCE_X, 0);
    custom_font.advance_y = (uint8_t) jerryxx_get_property_number(font, MSTR_GRAPHICS_ADVANCE_Y, 0);
    jerry_value_t bitmap = jerryxx_get_property(font, MSTR_GRAPHICS_BITMAP);
    // TODO: need to check bitmap is ArrayBuffer type
    custom_font.bitmap = jerry_get_arraybuffer_pointer(bitmap);
    jerry_release_value(bitmap);
    gc_handle->font = &custom_font;
  } else {
    gc_handle->font = NULL;
  }
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.setFontScale(scaleX, scaleY)
 */
JERRYXX_FUN(gc_set_font_scale_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "scaleX")
  JERRYXX_CHECK_ARG_NUMBER(1, "scaleY")
  int8_t scale_x = (int8_t) JERRYXX_GET_ARG_NUMBER(0);
  int8_t scale_y = (int8_t) JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_handle->font_scale_x = scale_x;
  gc_handle->font_scale_y = scale_y;
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.setPixel(x, y, color)
 */
JERRYXX_FUN(gc_set_pixel_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "color")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  uint8_t color = (uint8_t) JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_pixel(gc_handle, x, y, color);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.getPixel(x, y) -> color
 */
JERRYXX_FUN(gc_get_pixel_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_prim_get_pixel(gc_handle, x, y);
  return jerry_create_number(color);
}

/**
 * GraphicContext.prototype.drawLine(x0, y0, x1, y1)
 */
JERRYXX_FUN(gc_draw_line_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x0")
  JERRYXX_CHECK_ARG_NUMBER(1, "y0")
  JERRYXX_CHECK_ARG_NUMBER(2, "x1")
  JERRYXX_CHECK_ARG_NUMBER(3, "y1")
  int16_t x0 = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y0 = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t x1 = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  int16_t y1 = (int16_t) JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_line(gc_handle, x0, y0, x1, y1);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.drawRect(x, y, w, h)
 */
JERRYXX_FUN(gc_draw_rect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t) JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_rect(gc_handle, x, y, w, h);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.fillRect(x, y, w, h)
 */
JERRYXX_FUN(gc_fill_rect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t) JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_rect(gc_handle, x, y, w, h);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.drawCircle(x, y, r)
 */
JERRYXX_FUN(gc_draw_circle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "r")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t r = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_circle(gc_handle, x, y, r);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.fillCircle(x, y, r)
 */
JERRYXX_FUN(gc_fill_circle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "r")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t r = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_circle(gc_handle, x, y, r);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.drawRoundRect(x, y, w, h, r)
 */
JERRYXX_FUN(gc_draw_roundrect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  JERRYXX_CHECK_ARG_NUMBER(4, "r")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t) JERRYXX_GET_ARG_NUMBER(3);
  int16_t r = (int16_t) JERRYXX_GET_ARG_NUMBER(4);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_roundrect(gc_handle, x, y, w, h, r);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.fillRoundRect(x, y, w, h, r)
 */
JERRYXX_FUN(gc_fill_roundrect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  JERRYXX_CHECK_ARG_NUMBER(4, "r")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t) JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t) JERRYXX_GET_ARG_NUMBER(3);
  int16_t r = (int16_t) JERRYXX_GET_ARG_NUMBER(4);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_roundrect(gc_handle, x, y, w, h, r);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.drawText(x, y, text)
 */
JERRYXX_FUN(gc_draw_text_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_STRING(2, "text")
  int16_t x = (int16_t) JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t) JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_ARG_STRING_AS_CHAR(2, text)
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_text(gc_handle, x, y, text);
  return jerry_create_undefined();
}

/**
 * GraphicContext.prototype.flush() function
 */
JERRYXX_FUN(gc_flush_fn) {
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
  /* GraphicContext class */
  jerry_value_t gc_ctor = jerry_create_external_function(gc_ctor_fn);
  jerry_value_t gc_prototype = jerry_create_object();
  jerryxx_set_property(gc_ctor, "prototype", gc_prototype);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_CLEAR_SCREEN, gc_clear_screen_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_SCREEN, gc_fill_screen_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_ROTATION, gc_set_rotation_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_ROTATION, gc_get_rotation_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_WIDTH, gc_get_width_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_HEIGHT, gc_get_height_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_COLOR, gc_set_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_COLOR, gc_get_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FILL_COLOR, gc_set_fill_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_FILL_COLOR, gc_get_fill_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT_COLOR, gc_set_font_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_FONT_COLOR, gc_get_font_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT, gc_set_font_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT_SCALE, gc_set_font_scale_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_PIXEL, gc_set_pixel_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_PIXEL, gc_get_pixel_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_LINE, gc_draw_line_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_RECT, gc_draw_rect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_RECT, gc_fill_rect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_CIRCLE, gc_draw_circle_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_CIRCLE, gc_fill_circle_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_ROUNDRECT, gc_draw_roundrect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_ROUNDRECT, gc_fill_roundrect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_TEXT, gc_draw_text_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FLUSH, gc_flush_fn);
  jerry_release_value (gc_prototype);

  /* graphics module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_GRAPHICS_GRAPHIC_CONTEXT, gc_ctor);
  jerry_release_value (gc_ctor);

  return exports;
}
