#ifndef __GC_CB_PRIMS_H
#define __GC_CB_PRIMS_H

#include "font.h"
#include "gc.h"

// primitive functions for callback
void gc_prim_cb_set_pixel(gc_handle_t *handle, int16_t x, int16_t y,
                          uint16_t color);
void gc_prim_cb_get_pixel(gc_handle_t *handle, int16_t x, int16_t y,
                          uint16_t *color);
void gc_prim_cb_draw_vline(gc_handle_t *handle, int16_t x, int16_t y, int16_t h,
                           uint16_t color);
void gc_prim_cb_draw_hline(gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
                           uint16_t color);
void gc_prim_cb_fill_rect(gc_handle_t *handle, int16_t x, int16_t y, int16_t w,
                          int16_t h, uint16_t color);
void gc_prim_cb_fill_screen(gc_handle_t *handle, uint16_t color);

#endif /* __GC_CB_PRIMS_H */
