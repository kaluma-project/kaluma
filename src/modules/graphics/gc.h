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

#ifndef __GC_H
#define __GC_H

#include <stdlib.h>
#include "font.h"

/**
 * Graphic context native handle
 */
typedef struct {
  int16_t device_width;
  int16_t device_height;
  int16_t width;
  int16_t height;
  uint8_t rotation;
  uint8_t *buffer;
  uint16_t buffer_size;
  uint16_t color;
  uint16_t fill_color;
  gc_font_t *font;
  uint16_t font_color;
} gc_handle_t;

// primitive device-dependant functions
void gc_prim_set_pixel(gc_handle_t *handle, int16_t x, int16_t y, uint16_t color);
uint16_t gc_prim_get_pixel(gc_handle_t *handle, int16_t x, int16_t y);
void gc_prim_draw_fast_vline(gc_handle_t *handle, int16_t x, int16_t y, int16_t h, uint16_t color);
void gc_prim_draw_fast_hline(gc_handle_t *handle, int16_t x, int16_t y, int16_t w, uint16_t color);
void gc_prim_draw_line(gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gc_prim_fill_rect(gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gc_prim_fill_screen (gc_handle_t *handle, uint16_t color);

// graphic device-neutral functions
void gc_clear_screen (gc_handle_t *handle);
void gc_fill_screen (gc_handle_t *handle, uint16_t color);
void gc_set_rotation (gc_handle_t *handle, uint8_t rotation);
uint8_t gc_get_rotation (gc_handle_t *handle);
void gc_set_color (gc_handle_t *handle, uint16_t color);
uint16_t gc_get_color (gc_handle_t *handle);
void gc_set_fill_color (gc_handle_t *handle, uint16_t color);
uint16_t gc_get_fill_color (gc_handle_t *handle);
void gc_set_pixel (gc_handle_t *handle, int16_t x, int16_t y, uint16_t color);
uint16_t gc_get_pixel (gc_handle_t *handle, int16_t x, int16_t y);
void gc_draw_line (gc_handle_t *handle, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void gc_draw_rect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h);
void gc_draw_roundrect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r);
void gc_draw_circle (gc_handle_t *handle, int16_t x, int16_t y, int16_t r);
void gc_fill_rect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h);
void gc_fill_roundrect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r);
void gc_fill_circle (gc_handle_t *handle, int16_t x, int16_t y, int16_t r);

// void gc_draw_bitmap(gc_handle_t *handle, uint8_t bitmap[], int16_t w, int16_t h);

void gc_set_font_color(gc_handle_t *handle, uint16_t color);
uint16_t gc_get_font_color(gc_handle_t *handle);
void gc_set_font(gc_handle_t *handle, gc_font_t *font);
gc_font_t* gc_get_font(gc_handle_t *handle);
void gc_draw_char(gc_handle_t *handle, int16_t x, int16_t y, const char ch);
void gc_draw_text(gc_handle_t *handle, int16_t x, int16_t y, const char *text);

// void gc_get_text_bound(gc_handle_t *handle, const uint8_t *text, uint16_t *w, uint16_t *h);
// 
// 

#endif /* __GC_H */
