/* Copyright (c) 2017 Kalamu
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

#ifndef __FONT_H
#define __FONT_H

#include <stdlib.h>
#include <stdint.h>

/**
 * Font glyph
 */
typedef struct {
	uint8_t  width;
  uint8_t  height;
	uint8_t  advance_x;
} gc_font_glyph_t;

/**
 * Font struct
 */
typedef struct {
  uint8_t *bitmap;         // Glyph bitmap data
  gc_font_glyph_t *glyphs; // NULL if fixed-size font
  uint8_t first;           // First char (ASCII) in bitmap
  uint8_t last;            // Last char (ASCII) in bitmap
  uint8_t width;           // Glyph width in pixels if no glyph data
  uint8_t height;          // Glyph height in pixels if no glyph data
  uint8_t advance_x;       // Distance to next char
  uint8_t advance_y;       // Newline distance
} gc_font_t;

extern const uint8_t font_default_bitmap[];

#endif /* __FONT_H */
