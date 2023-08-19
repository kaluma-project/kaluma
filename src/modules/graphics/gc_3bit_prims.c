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

#include "gc_3bit_prims.h"

#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "gc.h"
#include "jerryscript.h"



/*
 * @brief  Convert RGB 5-6-5 to RGB 1-1-1
 * @param  color
 * @param  x
 * @param  y
 * @param  color
 */
static uint8_t color_to_3bit(uint16_t color) {
 return (color & 0xF800 ? 0x00 : 0x04) |
        (color & 0x07E0 ? 0x00 : 0x02) |
        (color & 0x001F ? 0x00 : 0x01); 
}

/*
 * @brief  Convert RGB 1-1-1 to RGB 5-6-5
 * @param  color
 * @param  x
 * @param  y
 * @param  color
 */
static uint16_t color_from_3bit(uint8_t color) {
 return (color & 0x04 ? 0x0000 : 0xF800 ) |
        (color & 0x02 ? 0x0000 : 0x07E0 ) |
        (color & 0x01 ? 0x0000 : 0x001F); 
}

/**
 * Graphic primitive functions for 1bit (mono) graphic buffer
 */

/**
 * +--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--+
 * |A7|A6|A5|A4|A3|A2|A1|A0||B7|B6|B5|B4|B3|B2|B1|B0|
 * +--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--+
 * |XX|XX|R0|G0|B0|R1|G1|B1||XX|XX|R2|G2|B2|R3|G3|B3|
 * +--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--+
 * @brief  Primitive set pixel
 * @param  handle  Graphic context handle
 * @param  x
 * @param  y
 * @param  color
 */
void gc_prim_3bit_set_pixel(gc_handle_t *handle, int16_t x, int16_t y,
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
    uint32_t idx = ((y * handle->device_width) + x) / 2;
    uint8_t convertedColor = color_to_3bit(color);
    bool highPixel = ((x & 1) != 0);
    uint8_t pixel = handle->buffer[idx] & (highPixel ? 0xF8 : 0xC7);
    handle->buffer[idx] = pixel | (highPixel ? (convertedColor) : (convertedColor << 3));
  }
}





/**
 * @brief  Get color of the pixel at position (x, y)
 * @param  x
 * @param  y
 * @param  color Returned color
 */
void gc_prim_3bit_get_pixel(gc_handle_t *handle, int16_t x, int16_t y,
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
    uint32_t idx = ((y * handle->device_width) + x) / 2;

    
    if ((x & 1) != 0) {
        *color = color_from_3bit(handle->buffer[idx] & 0x07);
    }
    else {
        *color = color_from_3bit((handle->buffer[idx] & 0x38) >> 3);
    }
    return;
  }
  *color = 0;
  return;
}

/**
 * @brief Primitive draw fast vertical line
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param h
 * @param color
 */
void gc_prim_3bit_draw_vline(gc_handle_t *handle, int16_t x, int16_t y,
                             int16_t h, uint16_t color) {
  for (int16_t i = y; i < y + h; i++) {
    gc_prim_3bit_set_pixel(handle, x, i, color);
  }
}

/**
 * @brief Primitive draw fast horizontal line
 * @param handle Graphic context handle
 * @param x
 * @param y
 * @param w
 * @param color
 */
void gc_prim_3bit_draw_hline(gc_handle_t *handle, int16_t x, int16_t y,
                             int16_t w, uint16_t color) {
  for (int16_t i = x; i < x + w; i++) {
    gc_prim_3bit_set_pixel(handle, i, y, color);
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
void gc_prim_3bit_fill_rect(gc_handle_t *handle, int16_t x, int16_t y,
                            int16_t w, int16_t h, uint16_t color) {
  for (int16_t i = x; i < x + w; i++) {
    gc_prim_3bit_draw_vline(handle, i, y, h, color);
  }
}

/**
 * @brief Primitive fill screen
 * @param handle Graphic context handle
 * @param color
 */
void gc_prim_3bit_fill_screen(gc_handle_t *handle, uint16_t color) {

  uint8_t tempColor = color_to_3bit(color);
  uint8_t fillColor = (tempColor) | ((tempColor) << 3);
  for (int16_t y = 0; y < handle->device_height; y++) {
    uint32_t idx = (y * handle->device_width) / 2;
    for (int16_t x = 0; x < (handle->device_width/2); x++) {
      handle->buffer[idx + x] = fillColor;
    }
  }

}
