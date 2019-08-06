#include <stdlib.h>

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

/*
  setFont (font) {}
  getFont () {}
  drawPolygon (pts) {}
  fillPolygon (pts) {}
  drawText(x, y, text) {}
  getTextBound(text) {} : returns {w, h}
  drawBitmap(x, y, bitmap, w, h) {}
*/
