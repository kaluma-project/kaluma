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

#include <stdlib.h>
#include <string.h>
#include "jerryscript.h"
#include "font.h"
#include "gc.h"
#include "gc_cb_prims.h"

/**
 * Graphic primitive functions for callback javascript functions
 */

void gc_prim_cb_set_pixel(gc_handle_t *handle, int16_t x, int16_t y, uint16_t color) {

}

void gc_prim_cb_get_pixel(gc_handle_t *handle, int16_t x, int16_t y, uint16_t *color) {

}

void gc_prim_cb_draw_vline(gc_handle_t *handle, int16_t x, int16_t y, int16_t h, uint16_t color) {

}

void gc_prim_cb_draw_hline(gc_handle_t *handle, int16_t x, int16_t y, int16_t w, uint16_t color) {

}

void gc_prim_cb_fill_rect(gc_handle_t *handle, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

}

void gc_prim_cb_fill_screen (gc_handle_t *handle, uint16_t color) {

}
