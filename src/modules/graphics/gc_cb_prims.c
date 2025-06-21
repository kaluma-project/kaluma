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
 *
 *
 */

#include "gc_cb_prims.h"

#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "gc.h"
#include "jerryscript.h"

/**
 * Graphic primitive functions for callback javascript functions
 */

void gc_prim_cb_set_pixel(gc_handle_t *handle, int16_t x, int16_t y,
                          uint16_t color) {
  if ((x >= 0) && (x < handle->width) && (y >= 0) && (y < handle->height)) {
    switch (handle->rotation) {
      case 1:
        SWAP_INT16(x, y)
        x = handle->device_width - x - 1;
        break;
      case 2:
        x = handle->device_width - x - 1;
        y = handle->device_height - y - 1;
        break;
      case 3:
        SWAP_INT16(x, y)
        y = handle->device_height - y - 1;
        break;
    }
    if (jerry_value_is_function(handle->set_pixel_js_cb)) {
      jerry_value_t this_val = jerry_undefined();
      jerry_value_t arg_x = jerry_number(x);
      jerry_value_t arg_y = jerry_number(y);
      jerry_value_t arg_color = jerry_number(color);
      jerry_value_t args[] = {arg_x, arg_y, arg_color};
      jerry_value_t ret_val =
          jerry_call(handle->set_pixel_js_cb, this_val, args, 3);
      jerry_value_free(ret_val);
      jerry_value_free(arg_x);
      jerry_value_free(arg_y);
      jerry_value_free(arg_color);
      jerry_value_free(this_val);
    }
  }
}

void gc_prim_cb_get_pixel(gc_handle_t *handle, int16_t x, int16_t y,
                          uint16_t *color) {
  if ((x >= 0) && (x < handle->width) && (y >= 0) && (y < handle->height)) {
    switch (handle->rotation) {
      case 1:
        SWAP_INT16(x, y)
        x = handle->device_width - x - 1;
        break;
      case 2:
        x = handle->device_width - x - 1;
        y = handle->device_height - y - 1;
        break;
      case 3:
        SWAP_INT16(x, y)
        y = handle->device_height - y - 1;
        break;
    }
    if (jerry_value_is_function(handle->get_pixel_js_cb)) {
      jerry_value_t this_val = jerry_undefined();
      jerry_value_t arg_x = jerry_number(x);
      jerry_value_t arg_y = jerry_number(y);
      jerry_value_t args[] = {arg_x, arg_y};
      jerry_value_t ret_val =
          jerry_call(handle->get_pixel_js_cb, this_val, args, 2);
      if (jerry_value_is_number(ret_val)) {
        *color = (uint16_t)jerry_value_as_number(ret_val);
      }
      jerry_value_free(ret_val);
      jerry_value_free(arg_x);
      jerry_value_free(arg_y);
      jerry_value_free(this_val);
    }
  }
}

void gc_prim_cb_draw_vline(gc_handle_t *handle, int16_t x, int16_t y, int16_t h,
                           uint16_t color) {
  gc_prim_cb_fill_rect(handle, x, y, 1, h, color);
}

void gc_prim_cb_draw_hline(gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
                           uint16_t color) {
  gc_prim_cb_fill_rect(handle, x, y, w, 1, color);
}

void gc_prim_cb_fill_rect(gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
                          int16_t h, uint16_t color) {
  int16_t x1 = x;
  int16_t y1 = y;
  int16_t x2 = x + w - 1;
  int16_t y2 = y + h - 1;
  // rotate
  switch (handle->rotation) {
    case 1:
      SWAP_INT16(x1, y1)
      SWAP_INT16(x2, y2)
      x1 = handle->device_width - x1 - 1;
      x2 = handle->device_width - x2 - 1;
      break;
    case 2:
      x1 = handle->device_width - x1 - 1;
      y1 = handle->device_height - y1 - 1;
      x2 = handle->device_width - x2 - 1;
      y2 = handle->device_height - y2 - 1;
      break;
    case 3:
      SWAP_INT16(x1, y1)
      SWAP_INT16(x2, y2)
      y1 = handle->device_height - y1 - 1;
      y2 = handle->device_height - y2 - 1;
      break;
  }
  if (x1 > x2) {
    SWAP_INT16(x1, x2)
  }
  if (y1 > y2) {
    SWAP_INT16(y1, y2)
  }
  // clipping
  if (x1 < 0) x1 = 0;
  if (x1 >= handle->device_width) x1 = handle->device_width - 1;
  if (y1 < 0) y1 = 0;
  if (y1 >= handle->device_height) y1 = handle->device_height - 1;
  if (x2 < 0) x2 = 0;
  if (x2 >= handle->device_width) x2 = handle->device_width - 1;
  if (y2 < 0) y2 = 0;
  if (y2 >= handle->device_height) y2 = handle->device_height - 1;
  // draw
  if (jerry_value_is_function(handle->fill_rect_js_cb)) {
    jerry_value_t this_val = jerry_undefined();
    jerry_value_t arg_x = jerry_number(x1);
    jerry_value_t arg_y = jerry_number(y1);
    jerry_value_t arg_w = jerry_number(x2 - x1 + 1);
    jerry_value_t arg_h = jerry_number(y2 - y1 + 1);
    jerry_value_t arg_color = jerry_number(color);
    jerry_value_t args[] = {arg_x, arg_y, arg_w, arg_h, arg_color};
    jerry_value_t ret_val =
        jerry_call(handle->fill_rect_js_cb, this_val, args, 5);
    jerry_value_free(ret_val);
    jerry_value_free(arg_x);
    jerry_value_free(arg_y);
    jerry_value_free(arg_w);
    jerry_value_free(arg_h);
    jerry_value_free(arg_color);
    jerry_value_free(this_val);
  }
}

void gc_prim_cb_fill_screen(gc_handle_t *handle, uint16_t color) {
  gc_prim_cb_fill_rect(handle, 0, 0, handle->device_width,
                       handle->device_height, color);
}
