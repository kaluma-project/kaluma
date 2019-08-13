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
 * 
 *
 */

/**
 * Many graphic function implementation is comes from Adafruit GFX library.
 * Thanks to Adafruit.
 */

#include <stdlib.h>
#include <string.h>
#include "jerryscript.h"
#include "font.h"
#include "gc.h"

#ifndef SWAP_INT16
#define SWAP_INT16(a, b) { int16_t t = a; a = b; b = t; }
#endif

/* ************************************************************************** */
/*                        PRIMITIVE GRAPHIC FUNCTIONS                         */
/* ************************************************************************** */

/**
 * @brief  Primitive set pixel
 * @param  handle  Graphic context handle
 * @param  x
 * @param  y
 * @param  color
 */
void gc_prim_set_pixel(gc_handle_t *handle, int16_t x, int16_t y,
    uint16_t color) {
  if((x >= 0) && (x < handle->width) && (y >= 0) && (y < handle->height)) {
    switch (handle->rotation) {
     case 1:
      SWAP_INT16(x, y)
      x = handle->device_width - x - 1;
      break;
     case 2:
      x = handle->device_width  - x - 1;
      y = handle->device_height - y - 1;
      break;
     case 3:
      SWAP_INT16(x, y)
      y = handle->device_height - y - 1;
      break;
    }
    uint16_t idx = x + (y / 8) * handle->device_width;
    uint8_t mask = (1 << (y & 7));
    if (color) {
      handle->buffer[idx] |= mask;
    } else {
      handle->buffer[idx] &= ~mask;
    }
  }
}

/**
 * @brief  Get olor of the pixel in the buffer
 * @param  x
 * @param  y
 * @return  color if pixel has color, 0 if pixel is outbound
 */
uint16_t gc_prim_get_pixel(gc_handle_t *handle, int16_t x, int16_t y) {
  if((x >= 0) && (x < handle->width) && (y >= 0) && (y < handle->height)) {
    switch (handle->rotation) {
     case 1:
      SWAP_INT16(x, y)
      x = handle->device_width - x - 1;
      break;
     case 2:
      x = handle->device_width  - x - 1;
      y = handle->device_height - y - 1;
      break;
     case 3:
      SWAP_INT16(x, y)
      y = handle->device_height - y - 1;
      break;
    }
    return (handle->buffer[x + (y / 8) * handle->device_width] & (1 << (y & 7))) > 0;
  }
  return 0;
}

/**
 * @brief Primitive draw fast vertical line
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param h
 * @param color
 */
void gc_prim_draw_fast_vline(gc_handle_t *handle, int16_t x, int16_t y,
    int16_t h, uint16_t color) {
  gc_prim_draw_line(handle, x, y, x, y + h - 1, color);
}

/**
 * @brief Primitive draw fast horizontal line
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param w
 * @param color
 */
void gc_prim_draw_fast_hline(gc_handle_t *handle, int16_t x, int16_t y,
    int16_t w, uint16_t color) {
  gc_prim_draw_line(handle, x, y, x + w - 1, y, color);
}

/**
 * @brief Primitive draw line (Bresenham's algorithm)
 * @param handle Graphic context handle
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 * @param color
 */
void gc_prim_draw_line(gc_handle_t *handle, int16_t x0, int16_t y0, int16_t x1,
    int16_t y1, uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    SWAP_INT16(x0, y0);
    SWAP_INT16(x1, y1);
  }
  if (x0 > x1) {
    SWAP_INT16(x0, x1);
    SWAP_INT16(y0, y1);
  }
  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);
  int16_t err = dx / 2;
  int16_t ystep;
  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }
  for (; x0<=x1; x0++) {
    if (steep) {
      gc_prim_set_pixel(handle, y0, x0, color);
    } else {
      gc_prim_set_pixel(handle, x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/**
 * @brief Primitive filled rectangle
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param w
 * @param h
 * @param color
 */
void gc_prim_fill_rect(gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
    int16_t h, uint16_t color) {
  for (int16_t i = x; i < x + w; i++) {
    gc_prim_draw_fast_vline(handle, i, y, h, color);
  }
}

/**
 * @brief Primitive fill screen
 * @param handle Graphic context handle
 * @param color
 */
void gc_prim_fill_screen (gc_handle_t *handle, uint16_t color) {
  for (int i = 0; i < handle->buffer_size; i++) {
    if (color) {
      handle->buffer[i] = 255;
    } else {
      handle->buffer[i] = 0;
    }
  }
}

/* ************************************************************************** */
/*                      GRAPHIC DEVICE_NEUTRAL FUNCTIONS                      */
/* ************************************************************************** */

int16_t gc_get_width (gc_handle_t *handle) {
  return handle->width;
}

int16_t gc_get_height (gc_handle_t *handle) {
  return handle->height;
}

/**
 * @brief Clear screen
 * @param handle Graphic context handle
 */
void gc_clear_screen (gc_handle_t *handle) {
  gc_prim_fill_screen(handle, 0);
}

/**
 * @brief Fill screen
 * @param handle Graphic context handle
 * @param color
 */
void gc_fill_screen (gc_handle_t *handle, uint16_t color) {
  gc_prim_fill_screen(handle, color);
}

/**
 * @brief  Set screen rotation
 * @param  handle    Graphic context handle
 * @param  rotation  0 thru 3 corresponding to 4 cardinal rotations
 */
void gc_set_rotation (gc_handle_t *handle, uint8_t rotation) {
  handle->rotation = (rotation & 3);
  switch (handle->rotation) {
    case 0:
    case 2:
      handle->width = handle->device_width;
      handle->height = handle->device_height;
      break;
    case 1:
    case 3:
      handle->width = handle->device_height;
      handle->height = handle->device_width;
      break;
  }
}

/**
 * @brief   Return screen rotation
 * @param   handle    Graphic context handle
 * @return  rotation  0 thru 3 corresponding to 4 cardinal rotations
 */
uint8_t gc_get_rotation (gc_handle_t *handle) {
  return handle->rotation;
}

/**
 * @brief Set stoke color
 * @param handle Graphic context handle
 * @param color
 */
void gc_set_color (gc_handle_t *handle, uint16_t color) {
  handle->color = color;
}

/**
 * @brief Get stoke color
 * @param handle Graphic context handle
 * @return Current stroke color
 */
uint16_t gc_get_color (gc_handle_t *handle) {
  return handle->color;
}

/**
 * @brief Set fill color
 * @param handle Graphic context handle
 * @param color
 */
void gc_set_fill_color (gc_handle_t *handle, uint16_t color) {
  handle->fill_color = color;
}

/**
 * @brief Get fill color
 * @param handle Graphic context handle
 * @return Current fill color
 */
uint16_t gc_get_fill_color (gc_handle_t *handle) {
  return handle->fill_color;
}

/**
 * @brief Set pixel
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param color
 */
void gc_set_pixel (gc_handle_t *handle, int16_t x, int16_t y, uint16_t color) {
  gc_prim_set_pixel(handle, x, y, color);
}

/**
 * @brief Get pixel
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @return Color at (x, y) coordinate
 */
uint16_t gc_get_pixel (gc_handle_t *handle, int16_t x, int16_t y) {
  return gc_prim_get_pixel(handle, x, y);
}

/**
 * @brief Draw line
 * @param handle Graphic context handle
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 */
void gc_draw_line (gc_handle_t *handle, int16_t x0, int16_t y0, int16_t x1,
    int16_t y1) {
  gc_prim_draw_line(handle, x0, y0, x1, y1, handle->color);
}

/**
 * @brief Draw rectangle
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param w
 * @param h
 */
void gc_draw_rect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
    int16_t h) {
  gc_prim_draw_fast_hline(handle, x, y, w, handle->color);
  gc_prim_draw_fast_hline(handle, x, y + h - 1, w, handle->color);
  gc_prim_draw_fast_vline(handle, x, y, h, handle->color);
  gc_prim_draw_fast_vline(handle, x + w - 1, y, h, handle->color);
}

/**
 * @brief  Quarter-circle drawer, used to do circles and roundrects
 * @param  x0          Center-point x coordinate
 * @param  y0          Center-point y coordinate
 * @param  r           Radius of circle
 * @param  cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
 * @param  color       16-bit 5-6-5 Color to draw with
 */
void gc_draw_circle_helper(gc_handle_t *handle, int16_t x, int16_t y, int16_t r,
    uint8_t cornername, uint16_t color) {
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t _x     = 0;
  int16_t _y     = r;

  while (_x < _y) {
    if (f >= 0) {
      _y--;
      ddF_y += 2;
      f += ddF_y;
    }
    _x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      gc_prim_set_pixel(handle, x + _x, y + _y, color);
      gc_prim_set_pixel(handle, x + _y, y + _x, color);
    }
    if (cornername & 0x2) {
      gc_prim_set_pixel(handle, x + _x, y - _y, color);
      gc_prim_set_pixel(handle, x + _y, y - _x, color);
    }
    if (cornername & 0x8) {
      gc_prim_set_pixel(handle, x - _y, y + _x, color);
      gc_prim_set_pixel(handle, x - _x, y + _y, color);
    }
    if (cornername & 0x1) {
      gc_prim_set_pixel(handle, x - _y, y - _x, color);
      gc_prim_set_pixel(handle, x - _x, y - _y, color);
    }
  }
}

/**
 * @brief  Draw rounded rectangle
 * @param  handle Graphic context handle
 * @param  x
 * @param  y
 * @param  w
 * @param  h
 * @param  r  Radius of corner rounding
 */
void gc_draw_roundrect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
    int16_t h, int16_t r) {
  int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
  if(r > max_radius) r = max_radius;
  // smarter version
  gc_prim_draw_fast_hline(handle, x + r, y, w - 2 * r, handle->color);
  gc_prim_draw_fast_hline(handle, x + r, y + h - 1, w - 2 * r, handle->color);
  gc_prim_draw_fast_vline(handle, x, y+r, h - 2 * r, handle->color);
  gc_prim_draw_fast_vline(handle, x + w - 1, y + r, h - 2 * r, handle->color);
  // draw four corners
  gc_draw_circle_helper(handle, x + r, y + r, r, 1, handle->color);
  gc_draw_circle_helper(handle, x + w - r - 1, y + r, r, 2, handle->color);
  gc_draw_circle_helper(handle, x + w - r - 1, y + h - r - 1, r, 4, handle->color);
  gc_draw_circle_helper(handle, x + r, y + h - r - 1, r, 8, handle->color);
}

/**
 * @brief Draw circle
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param r
 */
void gc_draw_circle (gc_handle_t *handle, int16_t x, int16_t y, int16_t r) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t _x = 0;
  int16_t _y = r;

  gc_prim_set_pixel(handle, x, y + r, handle->color);
  gc_prim_set_pixel(handle, x, y - r, handle->color);
  gc_prim_set_pixel(handle, x + r, y, handle->color);
  gc_prim_set_pixel(handle, x - r, y, handle->color);

  while (_x < _y) {
    if (f >= 0) {
        _y--;
        ddF_y += 2;
        f += ddF_y;
    }
    _x++;
    ddF_x += 2;
    f += ddF_x;
    gc_prim_set_pixel(handle, x + _x, y + _y, handle->color);
    gc_prim_set_pixel(handle, x - _x, y + _y, handle->color);
    gc_prim_set_pixel(handle, x + _x, y - _y, handle->color);
    gc_prim_set_pixel(handle, x - _x, y - _y, handle->color);
    gc_prim_set_pixel(handle, x + _y, y + _x, handle->color);
    gc_prim_set_pixel(handle, x - _y, y + _x, handle->color);
    gc_prim_set_pixel(handle, x + _y, y - _x, handle->color);
    gc_prim_set_pixel(handle, x - _y, y - _x, handle->color);
  }
}

/**
 * @brief Draw ellipse
 */
void gc_draw_ellipse (gc_handle_t *handle, int16_t x0, int16_t y0, int16_t x1,
    int16_t y1) {
}

/**
 * @brief Draw filled rectangle
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param w
 * @param h
 */
void gc_fill_rect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
    int16_t h) {
  gc_prim_fill_rect(handle, x, y, w, h, handle->fill_color);
}

/**
 * @brief Quarter-circle drawer with fill (from Adafruit GFX library)
 * @param  x
 * @param  y
 * @param  r
 * @param  corners  Mask bits indicating which quarters we're doing
 * @param  delta    Offset from center-point, used for round-rects
 * @param  color
 */
void gc_fill_circle_helper(gc_handle_t *handle, int16_t x, int16_t y, int16_t r,
    uint8_t corners, int16_t delta, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t _x = 0;
  int16_t _y = r;
  int16_t px = _x;
  int16_t py = _y;
  delta++;
  while(_x < _y) {
    if (f >= 0) {
      _y--;
      ddF_y += 2;
      f += ddF_y;
    }
    _x++;
    ddF_x += 2;
    f += ddF_x;
    if(_x < (_y + 1)) {
      if (corners & 1)
        gc_prim_draw_fast_vline(handle, x + _x, y - _y, 2 * _y + delta, color);
      if (corners & 2)
        gc_prim_draw_fast_vline(handle, x - _x, y - _y, 2 * _y + delta, color);
    }
    if(_y != py) {
      if (corners & 1)
        gc_prim_draw_fast_vline(handle, x + py, y - px, 2 * px + delta, color);
      if (corners & 2)
        gc_prim_draw_fast_vline(handle, x - py, y - px, 2 * px + delta, color);
      py = _y;
    }
    px = _x;
  }
}

/**
 * @brief  Draw filled rounded rectangle
 * @param  handle Graphic context handle
 * @param  x
 * @param  y
 * @param  w
 * @param  h
 * @param  r
 */
void gc_fill_roundrect (gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
    int16_t h, int16_t r) {
  int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
  if(r > max_radius) r = max_radius;
  gc_prim_fill_rect(handle, x + r, y, w - 2 * r, h, handle->fill_color);
  // draw four corners
  gc_fill_circle_helper(handle, x + w - r - 1, y + r, r, 1, h - 2 * r - 1, handle->fill_color);
  gc_fill_circle_helper(handle, x + r, y + r, r, 2, h - 2 * r - 1, handle->fill_color);
}

/**
 * @brief  Draw filled circle
 * @param  handle Graphic context handle
 * @param  x
 * @param  y
 * @param  r
 */
void gc_fill_circle (gc_handle_t *handle, int16_t x, int16_t y, int16_t r) {
  gc_prim_draw_fast_vline(handle, x, y - r, 2 * r + 1, handle->fill_color);
  gc_fill_circle_helper(handle, x, y, r, 3, 0, handle->fill_color);
}

/**
 * @brief Draw filled rectangle
 */
void gc_fill_ellipse (gc_handle_t *handle, int16_t x0, int16_t y0, int16_t x1,
    int16_t y1) {
}

/**
 * @brief
 */
void gc_set_font(gc_handle_t *handle, gc_font_t *font) {
  handle->font = font;
}

/**
 * @brief
 */
gc_font_t* gc_get_font(gc_handle_t *handle) {
  return handle->font;
}

/**
 * @brief
 */
void gc_set_font_color(gc_handle_t *handle, uint16_t color) {
  handle->font_color = color;
}

/**
 * @brief
 */
uint16_t gc_get_font_color(gc_handle_t *handle) {
  return handle->font_color;
}

/**
 * @brief
 */
void gc_set_font_scale(gc_handle_t *handle, uint8_t scale_x, uint8_t scale_y) {
  handle->font_scale_x = scale_x;
  handle->font_scale_y = scale_y;
}

/**
 * @brief
 */
void gc_draw_char(gc_handle_t *handle, int16_t x, int16_t y, const char ch) {
  uint8_t sx = handle->font_scale_x;
  uint8_t sy = handle->font_scale_y;
  if (handle->font == NULL) { /* default font */
    if ((x >= handle->width) || (y >= handle->height) ||
        ((x + 6 * sx - 1) < 0) || ((y + 8 * sy - 1) < 0))
      return;
    for (int8_t i = 0; i < 5; i++ ) {
      uint8_t line = font_default_bitmap[ch * 5 + i];
      for(int8_t j = 0; j < 8; j++, line >>= 1) {
        if (line & 1) {
          if(sx == 1 && sy == 1)
            gc_prim_set_pixel(handle, x + i, y + j, handle->font_color);
          else
            gc_prim_fill_rect(handle, x + i * sx, y + j * sy, sx,
                sy, handle->font_color);
        }
      }
    }
  } else {  /* custom font */
    uint8_t w = handle->font->width;
    uint8_t h = handle->font->height;
    uint16_t sz = (((w + 7) / 8) * h);
    uint16_t idx = ((uint8_t) ch) - handle->font->first;
    uint16_t offset = idx * sz;
    if ((x >= handle->width) || (y >= handle->height) ||
        ((x + w * sx - 1) < 0) || ((y + h * sy - 1) < 0))
      return;
    uint8_t bit = 0;
    uint8_t bits = handle->font->bitmap[offset];
    for(uint8_t yy = 0; yy < h; yy++) {
      for(uint8_t xx = 0; xx < w; xx++) {
        if (bit > 7) {
          bit = 0;
          offset++;
          bits = handle->font->bitmap[offset];
        }
        if (bits & 0x80) {
          if (sx == 1 && sy == 1) {
            gc_prim_set_pixel(handle, x + xx, y + yy, handle->font_color);
          } else {
            gc_prim_fill_rect(handle, x + xx * sx, y + yy * sy,
              sx, sy, handle->font_color);
          }
        }
        bits <<= 1;
        bit++;
      }
      offset++;
      bit = 0;
      bits = handle->font->bitmap[offset];
    }
  }
}

/**
 * @brief
 */
void gc_draw_text(gc_handle_t *handle, int16_t x, int16_t y, const char *text) {
  int16_t cursor_x = x;
  int16_t cursor_y = y;
  for (uint16_t i = 0; i < strlen(text); i++) {
    char ch = text[i];
    if (handle->font == NULL) {  /* default font */
      if (ch == '\n') {
        cursor_x = x;
        cursor_y += 8 * handle->font_scale_y;
      } else if (ch != '\r') {
        gc_draw_char(handle, cursor_x, cursor_y, ch);
        cursor_x += 6 * handle->font_scale_x;
      }
    } else {  /* custom font */
      if (ch == '\n') {
        cursor_x = x;
        cursor_y += handle->font->advance_y * handle->font_scale_y;
      } else if (ch != '\r') {
        gc_draw_char(handle, cursor_x, cursor_y, ch);
        if (handle->font->glyphs != NULL) {
          uint16_t idx = ((uint8_t) ch) - handle->font->first;
          gc_font_glyph_t glyph = handle->font->glyphs[idx];
          cursor_x += glyph.advance_x * handle->font_scale_x;
        } else {
          cursor_x += handle->font->advance_x * handle->font_scale_x;
        }
      }
    }
  }
}

void gc_measure_text(gc_handle_t *handle, const uint8_t *text, uint16_t *w,
    uint16_t *h) {
  // TODO: Implement when handling variable-width fonts
}

void gc_draw_bitmap(gc_handle_t *handle, int16_t x, int16_t y, uint8_t *bitmap,
    int16_t w, int16_t h, uint16_t color) {
  uint16_t offset = 0;
  if ((x >= handle->width) || (y >= handle->height) ||
      ((x + w - 1) < 0) || ((y + h - 1) < 0))
    return;
  uint8_t bit = 0;
  uint8_t bits = bitmap[offset];
  for(uint8_t yy = 0; yy < h; yy++) {
    for(uint8_t xx = 0; xx < w; xx++) {
      if (bit > 7) {
        bit = 0;
        offset++;
        bits = bitmap[offset];
      }
      if (bits & 0x80) {
        gc_prim_set_pixel(handle, x + xx, y + yy, color);
      }
      bits <<= 1;
      bit++;
    }
    offset++;
    bit = 0;
    bits = bitmap[offset];
  }
}
