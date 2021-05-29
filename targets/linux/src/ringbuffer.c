#include "ringbuffer.h"

void ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len) {
  ringbuffer->r_ptr = 0;
  ringbuffer->w_ptr = 0;
  ringbuffer->buf = buf;
  ringbuffer->length = len;
}

uint32_t ringbuffer_size(ringbuffer_t *ringbuffer) {
  return ringbuffer->length;
}

uint32_t ringbuffer_length(ringbuffer_t *ringbuffer) {
  uint32_t len;
  uint32_t w_ptr = ringbuffer->w_ptr;
  uint32_t r_ptr = ringbuffer->r_ptr;
  if (r_ptr <= w_ptr) {
    len = w_ptr - r_ptr;
  } else {
    len = w_ptr - r_ptr + ringbuffer->length;
  }
  return len;
}

uint32_t ringbuffer_freespace(ringbuffer_t *ringbuffer) {
  return (ringbuffer->length - ringbuffer_length(ringbuffer));
}

void ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len) {
  uint32_t k;
  uint32_t r_ptr;
  r_ptr = ringbuffer->r_ptr;
  for (k = 0; k < len; k++) {
    buf[k] = ringbuffer->buf[r_ptr];
    r_ptr = (r_ptr + 1) % ringbuffer->length;
  }
  ringbuffer->r_ptr = r_ptr;
}

void ringbuffer_write(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len) {
  uint32_t k;
  uint32_t w_ptr;
  w_ptr = ringbuffer->w_ptr;
  for (k = 0; k < len; k++) {
    ringbuffer->buf[w_ptr] = buf[k];
    w_ptr = (w_ptr + 1) % ringbuffer->length;
  }
  ringbuffer->w_ptr = w_ptr;
}

uint8_t ringbuffer_look_at(ringbuffer_t *ringbuffer, uint32_t offset) {
  uint32_t r_ptr;
  r_ptr = ringbuffer->r_ptr;
  r_ptr = (r_ptr + offset) % ringbuffer->length;
  return ringbuffer->buf[r_ptr];
}

void ringbuffer_look(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len,
                     uint32_t offset) {
  uint32_t k;
  uint32_t r_ptr;
  r_ptr = ringbuffer->r_ptr;
  r_ptr = (r_ptr + offset) % ringbuffer->length;
  for (k = 0; k < len; k++) {
    buf[k] = ringbuffer->buf[r_ptr];
    r_ptr = (r_ptr + 1) % ringbuffer->length;
  }
}

void ringbuffer_flush(ringbuffer_t *ringbuffer, uint32_t len) {
  uint32_t r_ptr;
  r_ptr = ringbuffer->r_ptr;
  r_ptr = (r_ptr + len) % ringbuffer->length;
  ringbuffer->r_ptr = r_ptr;
}

int ringbuffer_find(ringbuffer_t *ringbuffer, uint8_t ch) {
  uint32_t pos = -1;
  uint32_t len = ringbuffer_length(ringbuffer);
  uint32_t r_ptr;
  r_ptr = ringbuffer->r_ptr % ringbuffer->length;
  for (int n = 0; n < len; n++) {
    if (ringbuffer->buf[r_ptr] == ch) {
      pos = n;
      break;
    }
    r_ptr = (r_ptr + 1) % ringbuffer->length;
  }
  return pos;
}
