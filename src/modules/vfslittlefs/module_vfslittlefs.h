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

#include "jerryscript.h"

typedef struct vfs_littlefs_handle_s vfs_littlefs_handle_t;

/**
 * VFSLittleFS native handle
 */
struct vfs_littlefs_handle_s {
  // int16_t device_width;
  // int16_t device_width;
  // int16_t device_height;
  // int16_t width;
  // int16_t height;
  // uint8_t rotation;
  // uint8_t bpp;
  // uint8_t *buffer;
  // uint16_t buffer_size;
  // uint16_t color;
  // uint16_t fill_color;
  // gc_font_t *font;
  // uint16_t font_color;
  // uint8_t font_scale_x;
  // uint8_t font_scale_y;
  // gc_set_pixel_cb set_pixel_cb;
  // gc_get_pixel_cb get_pixel_cb;
  // gc_draw_hline_cb draw_hline_cb;
  // gc_draw_vline_cb draw_vline_cb;
  // gc_fill_rect_cb fill_rect_cb;
  // gc_fill_screen_cb fill_screen_cb;
  // jerry_value_t display_js_cb;
  // jerry_value_t set_pixel_js_cb;
  // jerry_value_t get_pixel_js_cb;
  // jerry_value_t fill_rect_js_cb;
};

jerry_value_t module_vfslittlefs_init();
