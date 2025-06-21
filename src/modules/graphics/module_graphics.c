/* Copyright (c) 2019 Kaluma
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

#include "module_graphics.h"

#include <stdlib.h>

#include "font.h"
#include "gc.h"
#include "gc_16bit_prims.h"
#include "gc_3bit_prims.h"
#include "gc_1bit_prims.h"
#include "gc_cb_prims.h"
#include "graphics_magic_strings.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"

gc_font_t custom_font;

static void gc_handle_freecb(void *handle, struct jerry_object_native_info_t *info_p) { free(handle); }

static const jerry_object_native_info_t gc_handle_info = {.free_cb =
                                                              gc_handle_freecb};

/* ************************************************************************** */
/*                            GRAPHIC CONTEXT CLASS                           */
/* ************************************************************************** */

/**
 * GraphicsContext() constructor
 */
JERRYXX_FUN(gc_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "width");
  JERRYXX_CHECK_ARG_NUMBER(1, "height");
  JERRYXX_CHECK_ARG_OBJECT_OPT(2, "options");

  // set native handle
  gc_handle_t *gc_handle = (gc_handle_t *)malloc(sizeof(gc_handle_t));
  gc_handle->color = 1;
  gc_handle->fill_color = 1;
  gc_handle->font = NULL;
  gc_handle->font_color = 1;
  gc_handle->font_scale_x = 1;
  gc_handle->font_scale_y = 1;
  jerry_object_set_native_ptr(JERRYXX_GET_THIS, &gc_handle_info, gc_handle);

  // read parameters
  gc_handle->device_width = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  gc_handle->device_height = (int16_t)JERRYXX_GET_ARG_NUMBER(1);

  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t options = JERRYXX_GET_ARG(2);
    if (jerry_value_is_object(options)) {
      // rotation
      uint8_t rotation = (uint8_t)jerryxx_get_property_number(
          options, MSTR_GRAPHICS_ROTATION, 0);
      gc_set_rotation(gc_handle, rotation);

      // setPixel callback
      jerry_value_t set_pixel_js_cb =
          jerryxx_get_property(options, MSTR_GRAPHICS_SET_PIXEL);
      jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_SETPIXEL_CB,
                           set_pixel_js_cb);
      gc_handle->set_pixel_js_cb =
          set_pixel_js_cb;  // reference without acquire
      jerry_value_free(set_pixel_js_cb);

      // getPixel callback
      jerry_value_t get_pixel_js_cb =
          jerryxx_get_property(options, MSTR_GRAPHICS_GET_PIXEL);
      jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_GETPIXEL_CB,
                           get_pixel_js_cb);
      gc_handle->get_pixel_js_cb =
          get_pixel_js_cb;  // reference without acquire
      jerry_value_free(get_pixel_js_cb);

      // fillRect callback
      jerry_value_t fill_rect_js_cb =
          jerryxx_get_property(options, MSTR_GRAPHICS_FILL_RECT);
      jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_FILLRECT_CB,
                           fill_rect_js_cb);
      gc_handle->fill_rect_js_cb =
          fill_rect_js_cb;  // reference without acquire
      jerry_value_free(fill_rect_js_cb);
    }
  }

  // setup primitive functions
  gc_handle->set_pixel_cb = gc_prim_cb_set_pixel;
  gc_handle->get_pixel_cb = gc_prim_cb_get_pixel;
  gc_handle->draw_hline_cb = gc_prim_cb_draw_hline;
  gc_handle->draw_vline_cb = gc_prim_cb_draw_vline;
  gc_handle->fill_rect_cb = gc_prim_cb_fill_rect;
  gc_handle->fill_screen_cb = gc_prim_cb_fill_screen;

  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getWidth()
 */
JERRYXX_FUN(gc_get_width_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  int16_t width = gc_get_width(gc_handle);
  return jerry_number(width);
}

/**
 * GraphicsContext.prototype.getHeight()
 */
JERRYXX_FUN(gc_get_height_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  int16_t height = gc_get_height(gc_handle);
  return jerry_number(height);
}

/**
 * GraphicsContext.prototype.color16(r, g, b)
 */
JERRYXX_FUN(gc_color16_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "red")
  JERRYXX_CHECK_ARG_NUMBER(1, "green")
  JERRYXX_CHECK_ARG_NUMBER(2, "blue")
  uint8_t red = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t green = (uint8_t)JERRYXX_GET_ARG_NUMBER(1);
  uint8_t blue = (uint8_t)JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color16 = gc_color16(gc_handle, red, green, blue);
  return jerry_number(color16);
}

/**
 * GraphicsContext.prototype.clearScreen()
 */
JERRYXX_FUN(gc_clear_screen_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_clear_screen(gc_handle);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.fillScreen(color)
 */
JERRYXX_FUN(gc_fill_screen_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint16_t color = (uint16_t)JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_screen(gc_handle, color);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.setRotation(rotation)
 */
JERRYXX_FUN(gc_set_rotation_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "rotation")
  uint8_t rotation = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_rotation(gc_handle, rotation);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getRotation() -> rotation
 */
JERRYXX_FUN(gc_get_rotation_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint8_t rotation = gc_get_rotation(gc_handle);
  return jerry_number(rotation);
}

/**
 * GraphicsContext.prototype.setColor(color)
 */
JERRYXX_FUN(gc_set_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint16_t color = (uint16_t)JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_color(gc_handle, color);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getColor()
 */
JERRYXX_FUN(gc_get_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_color(gc_handle);
  return jerry_number(color);
}

/**
 * GraphicsContext.prototype.setFillColor(color)
 */
JERRYXX_FUN(gc_set_fill_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint16_t color = (uint16_t)JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_fill_color(gc_handle, color);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getFillColor()
 */
JERRYXX_FUN(gc_get_fill_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_fill_color(gc_handle);
  return jerry_number(color);
}

/**
 * GraphicsContext.prototype.setFontColor(color)
 */
JERRYXX_FUN(gc_set_font_color_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "color")
  uint16_t color = (uint16_t)JERRYXX_GET_ARG_NUMBER(0);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_font_color(gc_handle, color);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getFontColor()
 */
JERRYXX_FUN(gc_get_font_color_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_font_color(gc_handle);
  return jerry_number(color);
}

/**
 * GraphicsContext.prototype.setFont(font)
 */
JERRYXX_FUN(gc_set_font_fn) {
  JERRYXX_CHECK_ARG_OBJECT_NULL_OPT(0, "font")
  if (JERRYXX_HAS_ARG(0)) {
    jerry_value_t font = JERRYXX_GET_ARG(0);
    JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
    if (jerry_value_is_object(font)) {
      custom_font.first =
          (uint8_t)jerryxx_get_property_number(font, MSTR_GRAPHICS_FIRST, 0);
      custom_font.last =
          (uint8_t)jerryxx_get_property_number(font, MSTR_GRAPHICS_LAST, 0);
      custom_font.width =
          (uint8_t)jerryxx_get_property_number(font, MSTR_GRAPHICS_WIDTH, 0);
      custom_font.height =
          (uint8_t)jerryxx_get_property_number(font, MSTR_GRAPHICS_HEIGHT, 0);
      custom_font.advance_x = (uint8_t)jerryxx_get_property_number(
          font, MSTR_GRAPHICS_ADVANCE_X, 0);
      custom_font.advance_y = (uint8_t)jerryxx_get_property_number(
          font, MSTR_GRAPHICS_ADVANCE_Y, 0);
      // get bitmap buffer
      jerry_value_t bitmap = jerryxx_get_property(font, MSTR_GRAPHICS_BITMAP);
      if (jerry_value_is_typedarray(bitmap) &&
          jerry_typedarray_type(bitmap) ==
              JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
        jerry_length_t byteLength = 0;
        jerry_length_t byteOffset = 0;
        jerry_value_t buffer =
            jerry_typedarray_buffer(bitmap, &byteOffset, &byteLength);
        custom_font.bitmap = jerry_arraybuffer_data(buffer);
        jerry_value_free(buffer);
        // } else if (jerry_value_is_string(bitmap)) {
        //   custom_font.bitmap = NULL;
      } else {
        return jerry_error_sz(
            JERRY_ERROR_TYPE,
            "font.bitmap must be Uint8Array.");
      }
      jerry_value_free(bitmap);
      // get glyphs buffer
      jerry_value_t glyphs = jerryxx_get_property(font, MSTR_GRAPHICS_GLYPHS);
      if (jerry_value_is_typedarray(glyphs) &&
          jerry_typedarray_type(glyphs) == JERRY_TYPEDARRAY_UINT8) {
        jerry_length_t byteLength = 0;
        jerry_length_t byteOffset = 0;
        jerry_value_t buffer =
            jerry_typedarray_buffer(glyphs, &byteOffset, &byteLength);
        custom_font.glyphs =
            (gc_font_glyph_t *)jerry_arraybuffer_data(buffer);
        jerry_value_free(buffer);
        // } else if (jerry_value_is_string(glyphs)) {
        //   custom_font.glyphs = NULL;
      }
      jerry_value_free(glyphs);
      gc_handle->font = &custom_font;
    } else {
      gc_handle->font = NULL;
    }
  }
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.setFontScale(scaleX, scaleY)
 */
JERRYXX_FUN(gc_set_font_scale_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "scaleX")
  JERRYXX_CHECK_ARG_NUMBER(1, "scaleY")
  int8_t scale_x = (int8_t)JERRYXX_GET_ARG_NUMBER(0);
  int8_t scale_y = (int8_t)JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_font_scale(gc_handle, scale_x, scale_y);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.setPixel(x, y, color)
 */
JERRYXX_FUN(gc_set_pixel_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "color")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t color = (uint16_t)JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_set_pixel(gc_handle, x, y, color);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.getPixel(x, y) -> color
 */
JERRYXX_FUN(gc_get_pixel_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t color = gc_get_pixel(gc_handle, x, y);
  return jerry_number(color);
}

/**
 * GraphicsContext.prototype.drawLine(x0, y0, x1, y1)
 */
JERRYXX_FUN(gc_draw_line_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x0")
  JERRYXX_CHECK_ARG_NUMBER(1, "y0")
  JERRYXX_CHECK_ARG_NUMBER(2, "x1")
  JERRYXX_CHECK_ARG_NUMBER(3, "y1")
  int16_t x0 = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y0 = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t x1 = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  int16_t y1 = (int16_t)JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_line(gc_handle, x0, y0, x1, y1);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.drawRect(x, y, w, h)
 */
JERRYXX_FUN(gc_draw_rect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t)JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_rect(gc_handle, x, y, w, h);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.fillRect(x, y, w, h)
 */
JERRYXX_FUN(gc_fill_rect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t)JERRYXX_GET_ARG_NUMBER(3);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_rect(gc_handle, x, y, w, h);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.drawCircle(x, y, r)
 */
JERRYXX_FUN(gc_draw_circle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "r")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t r = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_circle(gc_handle, x, y, r);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.fillCircle(x, y, r)
 */
JERRYXX_FUN(gc_fill_circle_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "r")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t r = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_circle(gc_handle, x, y, r);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.drawRoundRect(x, y, w, h, r)
 */
JERRYXX_FUN(gc_draw_roundrect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  JERRYXX_CHECK_ARG_NUMBER(4, "r")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t)JERRYXX_GET_ARG_NUMBER(3);
  int16_t r = (int16_t)JERRYXX_GET_ARG_NUMBER(4);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_roundrect(gc_handle, x, y, w, h, r);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.fillRoundRect(x, y, w, h, r)
 */
JERRYXX_FUN(gc_fill_roundrect_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_NUMBER(2, "w")
  JERRYXX_CHECK_ARG_NUMBER(3, "h")
  JERRYXX_CHECK_ARG_NUMBER(4, "r")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  int16_t w = (int16_t)JERRYXX_GET_ARG_NUMBER(2);
  int16_t h = (int16_t)JERRYXX_GET_ARG_NUMBER(3);
  int16_t r = (int16_t)JERRYXX_GET_ARG_NUMBER(4);
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_fill_roundrect(gc_handle, x, y, w, h, r);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.drawText(x, y, text)
 */
JERRYXX_FUN(gc_draw_text_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  JERRYXX_CHECK_ARG_STRING(2, "text")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_ARG_STRING_AS_CHAR(2, text)
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  gc_draw_text(gc_handle, x, y, text);
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.measureText(text) -> {width, height}
 */
JERRYXX_FUN(gc_measure_text_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "text")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, text)
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  uint16_t w = 0;
  uint16_t h = 0;
  gc_measure_text(gc_handle, text, &w, &h);
  jerry_value_t metric = jerry_object();
  jerryxx_set_property_number(metric, "width", w);
  jerryxx_set_property_number(metric, "height", h);
  return metric;
}

/**
 * GraphicsContext.prototype.drawBitmap(x, y, bitmap, options)
 * - bitmap: {width, height, data, bpp}
 * - options: {color, transparent, scaleX, scaleY}
 */
JERRYXX_FUN(gc_draw_bitmap_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "x")
  JERRYXX_CHECK_ARG_NUMBER(1, "y")
  int16_t x = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  int16_t y = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  uint16_t w = 0;
  uint16_t h = 0;
  uint8_t bpp = 1;
  uint16_t color = bpp == 1 ? 1 : 0xffff;
  bool transparent = false;
  uint16_t transparent_color = 0;
  uint8_t scale_x = 1;
  uint8_t scale_y = 1;
  bool flip_x = false;
  bool flip_y = false;

  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t bitmap = JERRYXX_GET_ARG(2);
    if (jerry_value_is_object(bitmap)) {
      w = (uint16_t)jerryxx_get_property_number(bitmap, MSTR_GRAPHICS_WIDTH, w);
      h = (uint16_t)jerryxx_get_property_number(bitmap, MSTR_GRAPHICS_HEIGHT,
                                                h);
      bpp =
          (uint8_t)jerryxx_get_property_number(bitmap, MSTR_GRAPHICS_BPP, bpp);

      // get options
      if (JERRYXX_HAS_ARG(3)) {
        jerry_value_t options = JERRYXX_GET_ARG(3);
        if (jerry_value_is_object(options)) {
          color =
              jerryxx_get_property_number(options, MSTR_GRAPHICS_COLOR, color);
          jerry_value_t tp =
              jerryxx_get_property(options, MSTR_GRAPHICS_TRANSPARENT);
          if (jerry_value_is_number(tp)) {
            transparent = true;
            transparent_color = (uint16_t)jerry_value_as_number(tp);
          }
          jerry_value_free(tp);
          scale_x = jerryxx_get_property_number(options, MSTR_GRAPHICS_SCALE_X,
                                                scale_x);
          scale_y = jerryxx_get_property_number(options, MSTR_GRAPHICS_SCALE_Y,
                                                scale_y);
          flip_x = jerryxx_get_property_boolean(options, MSTR_GRAPHICS_FLIP_X,
                                                flip_x);
          flip_y = jerryxx_get_property_boolean(options, MSTR_GRAPHICS_FLIP_Y,
                                                flip_y);
        }
      }

      // draw bitmap
      JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
      jerry_value_t data = jerryxx_get_property(bitmap, MSTR_GRAPHICS_DATA);
      if (jerry_value_is_typedarray(data) &&
          jerry_typedarray_type(data) ==
              JERRY_TYPEDARRAY_UINT8) { /* Uint8Array */
        jerry_length_t byteLength = 0;
        jerry_length_t byteOffset = 0;
        jerry_value_t buffer =
            jerry_typedarray_buffer(data, &byteOffset, &byteLength);
        uint8_t *buf = jerry_arraybuffer_data(buffer);
        gc_draw_bitmap(gc_handle, x, y, buf, w, h, bpp, color, transparent,
                       transparent_color, scale_x, scale_y, flip_x, flip_y);
        jerry_value_free(buffer);
      } else if (jerry_value_is_string(data)) { /* decode base64 string */
        jerry_value_t global = jerry_current_realm();
        jerry_value_t atob_fn = jerryxx_get_property(global, MSTR_ATOB);
        jerry_value_t this_val = jerry_undefined();
        jerry_value_t args[] = {data};
        jerry_value_t decoded = jerry_call(atob_fn, this_val, args, 1);
        jerry_length_t byteLength = 0;
        jerry_length_t byteOffset = 0;
        jerry_value_t buffer =
            jerry_typedarray_buffer(decoded, &byteOffset, &byteLength);
        uint8_t *buf = jerry_arraybuffer_data(buffer);
        gc_draw_bitmap(gc_handle, x, y, buf, w, h, bpp, color, transparent,
                       transparent_color, scale_x, scale_y, flip_x, flip_y);
        jerry_value_free(buffer);
        jerry_value_free(decoded);
        jerry_value_free(this_val);
        jerry_value_free(atob_fn);
        jerry_value_free(global);
      } else {
        return jerry_error_sz(
            JERRY_ERROR_TYPE,
            "bitmap.data must be Uint8Array or string.");
      }
      jerry_value_free(data);
    }
  }
  return jerry_undefined();
}

/**
 * GraphicsContext.prototype.display() function
 */
JERRYXX_FUN(gc_display_fn) {
  JERRYXX_GET_NATIVE_HANDLE(gc_handle, gc_handle_t, gc_handle_info);
  if (jerry_value_is_function(gc_handle->display_js_cb)) {
    jerry_value_t buffer =
        jerryxx_get_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER);
    jerry_value_t this_ = jerry_undefined();
    jerry_value_t args[] = {buffer};
    jerry_value_t ret_val =
        jerry_call(gc_handle->display_js_cb, this_, args, 1);
    jerry_value_free(buffer);
    jerry_value_free(this_);
    return ret_val;
  } else {
    return jerry_undefined();
  }
}

/* ************************************************************************** */
/*                       BUFFERED GRAPHIC CONTEXT CLASS                       */
/* ************************************************************************** */

/**
 * BufferedGraphicsContext() constructor
 */
JERRYXX_FUN(buffered_gc_ctor_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "width");
  JERRYXX_CHECK_ARG_NUMBER(1, "height");
  JERRYXX_CHECK_ARG_OBJECT_OPT(2, "options");

  // set native handle
  gc_handle_t *gc_handle = (gc_handle_t *)malloc(sizeof(gc_handle_t));
  gc_handle->color = 1;
  gc_handle->fill_color = 1;
  gc_handle->font = NULL;
  gc_handle->font_color = 1;
  gc_handle->font_scale_x = 1;
  gc_handle->font_scale_y = 1;
  jerry_object_set_native_ptr(JERRYXX_GET_THIS, &gc_handle_info, gc_handle);

  // read parameters
  gc_handle->device_width = (int16_t)JERRYXX_GET_ARG_NUMBER(0);
  gc_handle->device_height = (int16_t)JERRYXX_GET_ARG_NUMBER(1);
  if (JERRYXX_HAS_ARG(2)) {
    jerry_value_t options = JERRYXX_GET_ARG(2);
    if (jerry_value_is_object(options)) {
      // rotation
      uint8_t rotation = (uint8_t)jerryxx_get_property_number(
          options, MSTR_GRAPHICS_ROTATION, 0);
      gc_set_rotation(gc_handle, rotation);

      // bpp
      uint8_t bpp = (uint8_t)jerryxx_get_property_number(
          options, MSTR_GRAPHICS_BPP, 1);  // should be 1 or 16
      if (bpp > 3) {
        gc_handle->bpp = 16;
      } else if (bpp > 1) {
        gc_handle->bpp = 3;
      } else {
        gc_handle->bpp = 1;
      }

      // display callback
      jerry_value_t display_js_cb =
          jerryxx_get_property(options, MSTR_GRAPHICS_DISPLAY);
      jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_DISPLAY_CB,
                           display_js_cb);
      gc_handle->display_js_cb = display_js_cb;  // reference without acquire
      jerry_value_free(display_js_cb);
    }
  }

  // setup primitive functions
  if (gc_handle->bpp == 1) {
    gc_handle->set_pixel_cb = gc_prim_1bit_set_pixel;
    gc_handle->get_pixel_cb = gc_prim_1bit_get_pixel;
    gc_handle->draw_hline_cb = gc_prim_1bit_draw_hline;
    gc_handle->draw_vline_cb = gc_prim_1bit_draw_vline;
    gc_handle->fill_rect_cb = gc_prim_1bit_fill_rect;
    gc_handle->fill_screen_cb = gc_prim_1bit_fill_screen;
  } else if (gc_handle->bpp == 3) {
    gc_handle->set_pixel_cb = gc_prim_3bit_set_pixel;
    gc_handle->get_pixel_cb = gc_prim_3bit_get_pixel;
    gc_handle->draw_hline_cb = gc_prim_3bit_draw_hline;
    gc_handle->draw_vline_cb = gc_prim_3bit_draw_vline;
    gc_handle->fill_rect_cb = gc_prim_3bit_fill_rect;
    gc_handle->fill_screen_cb = gc_prim_3bit_fill_screen;
  } else {
    gc_handle->set_pixel_cb = gc_prim_16bit_set_pixel;
    gc_handle->get_pixel_cb = gc_prim_16bit_get_pixel;
    gc_handle->draw_hline_cb = gc_prim_16bit_draw_hline;
    gc_handle->draw_vline_cb = gc_prim_16bit_draw_vline;
    gc_handle->fill_rect_cb = gc_prim_16bit_fill_rect;
    gc_handle->fill_screen_cb = gc_prim_16bit_fill_screen;
  }

  // allocate buffer
  size_t size = gc_handle->device_width * gc_handle->device_height;
  if (gc_handle->bpp == 1) {
    size = size / 8;
  } else if (gc_handle->bpp == 3) {
    size = size / 2;
  } else {
    size = size * 2;
  }
  jerry_value_t buffer = jerry_typedarray(JERRY_TYPEDARRAY_UINT8, size);
  jerryxx_set_property(JERRYXX_GET_THIS, MSTR_GRAPHICS_BUFFER, buffer);
  jerry_length_t byteOffset = 0;
  jerry_length_t byteLength = 0;
  jerry_value_t buf =
      jerry_typedarray_buffer(buffer, &byteOffset, &byteLength);
  gc_handle->buffer = jerry_arraybuffer_data(buf);
  gc_handle->buffer_size = size;
  jerry_value_free(buf);
  jerry_value_free(buffer);
  return jerry_undefined();
}

/* -------------------------------------------------------------------------- */

/**
 * Initialize 'graphics' module
 */
jerry_value_t module_graphics_init() {
  /* GraphicsContext class */
  jerry_value_t gc_ctor = jerry_function_external(gc_ctor_fn);
  jerry_value_t gc_prototype = jerry_object();
  jerryxx_set_property(gc_ctor, "prototype", gc_prototype);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_CLEAR_SCREEN,
                                gc_clear_screen_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_SCREEN,
                                gc_fill_screen_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_ROTATION,
                                gc_set_rotation_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_ROTATION,
                                gc_get_rotation_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_WIDTH,
                                gc_get_width_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_HEIGHT,
                                gc_get_height_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_COLOR16,
                                gc_color16_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_COLOR,
                                gc_set_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_COLOR,
                                gc_get_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FILL_COLOR,
                                gc_set_fill_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_FILL_COLOR,
                                gc_get_fill_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT_COLOR,
                                gc_set_font_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_FONT_COLOR,
                                gc_get_font_color_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT,
                                gc_set_font_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_FONT_SCALE,
                                gc_set_font_scale_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_SET_PIXEL,
                                gc_set_pixel_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_GET_PIXEL,
                                gc_get_pixel_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_LINE,
                                gc_draw_line_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_RECT,
                                gc_draw_rect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_RECT,
                                gc_fill_rect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_CIRCLE,
                                gc_draw_circle_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_CIRCLE,
                                gc_fill_circle_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_ROUNDRECT,
                                gc_draw_roundrect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_FILL_ROUNDRECT,
                                gc_fill_roundrect_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_TEXT,
                                gc_draw_text_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_MEASURE_TEXT,
                                gc_measure_text_fn);
  jerryxx_set_property_function(gc_prototype, MSTR_GRAPHICS_DRAW_BITMAP,
                                gc_draw_bitmap_fn);
  jerry_value_free(gc_prototype);

  /* BufferedGraphicsContext */
  jerry_value_t buffered_gc_ctor =
      jerry_function_external(buffered_gc_ctor_fn);
  jerry_value_t buffered_gc_prototype = jerry_object();
  jerryxx_set_property(buffered_gc_ctor, "prototype", buffered_gc_prototype);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_CLEAR_SCREEN, gc_clear_screen_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_FILL_SCREEN, gc_fill_screen_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_SET_ROTATION, gc_set_rotation_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_GET_ROTATION, gc_get_rotation_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_GET_WIDTH,
                                gc_get_width_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_GET_HEIGHT,
                                gc_get_height_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_COLOR16,
                                gc_color16_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_SET_COLOR,
                                gc_set_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_GET_COLOR,
                                gc_get_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_SET_FILL_COLOR,
                                gc_set_fill_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_GET_FILL_COLOR,
                                gc_get_fill_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_SET_FONT_COLOR,
                                gc_set_font_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_GET_FONT_COLOR,
                                gc_get_font_color_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_SET_FONT,
                                gc_set_font_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_SET_FONT_SCALE,
                                gc_set_font_scale_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_SET_PIXEL,
                                gc_set_pixel_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_GET_PIXEL,
                                gc_get_pixel_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_DRAW_LINE,
                                gc_draw_line_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_DRAW_RECT,
                                gc_draw_rect_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_FILL_RECT,
                                gc_fill_rect_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_DRAW_CIRCLE, gc_draw_circle_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_FILL_CIRCLE, gc_fill_circle_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_DRAW_ROUNDRECT,
                                gc_draw_roundrect_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_FILL_ROUNDRECT,
                                gc_fill_roundrect_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_DRAW_TEXT,
                                gc_draw_text_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_MEASURE_TEXT, gc_measure_text_fn);
  jerryxx_set_property_function(buffered_gc_prototype,
                                MSTR_GRAPHICS_DRAW_BITMAP, gc_draw_bitmap_fn);
  jerryxx_set_property_function(buffered_gc_prototype, MSTR_GRAPHICS_DISPLAY,
                                gc_display_fn);
  jerry_value_free(buffered_gc_prototype);

  /* graphics module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_GRAPHICS_GRAPHICS_CONTEXT, gc_ctor);
  jerryxx_set_property(exports, MSTR_GRAPHICS_BUFFERED_GRAPHICS_CONTEXT,
                       buffered_gc_ctor);
  jerry_value_free(gc_ctor);
  jerry_value_free(buffered_gc_ctor);

  return exports;
}
