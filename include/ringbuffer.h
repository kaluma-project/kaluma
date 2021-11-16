#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include <stdint.h>

typedef struct {
  uint8_t *buf;
  uint32_t length;
  uint32_t r_ptr;
  uint32_t w_ptr;
} ringbuffer_t;

/**
 * Initialize a ringbuffer with a given alocated buffer.
 *
 * @param ringbuffer
 * @param pbuf pointer to internal buffer
 * @param len length of internal buffer
 */
void ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len);

/**
 * Return the size of ring buffer.
 *
 * @param ringbuffer
 * @return size of the ringbuffer
 */
uint32_t ringbuffer_size(ringbuffer_t *ringbuffer);

/**
 * Return the length of data in the ring buffer.
 *
 * @param ringbuffer
 * @return size of the ringbuffer
 */
uint32_t ringbuffer_length(ringbuffer_t *ringbuffer);

/**
 * Return the free space of the ring buffer.
 *
 * @param ringbuffer
 * @return size of free space in the ringbuffer
 */
uint32_t ringbuffer_freespace(ringbuffer_t *ringbuffer);

/**
 * Read out data from the ring buffer.
 *
 * @param ringbuffer
 * @param buf buffer to store data read.
 * @param len Amount of data to read.
 */
void ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len);

/**
 * Write into data from the ring buffer.
 *
 * @param ringbuffer
 * @param buf data to write.
 * @param len size of data to write.
 */
void ringbuffer_write(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len);

/**
 * Look a character at the specified position in the ring buffer.
 *
 * @param ringbuffer
 * @param offset position to look at.
 * @return a character at the position.
 */
uint8_t ringbuffer_look_at(ringbuffer_t *ringbuffer, uint32_t offset);

/**
 * Look data at the specified position and length in the ring buffer.
 *
 * @param ringbuffer
 * @param buf buffer to copy the looked data.
 * @param len length of data to look.
 * @param offset position to start to look at.
 */
void ringbuffer_look(ringbuffer_t *ringbuffer, uint8_t *buf, uint32_t len,
                     uint32_t offset);

/**
 * Flush ring buffer.
 *
 * @param ringbuffer
 * @param len.
 */
void ringbuffer_flush(ringbuffer_t *ringbuffer, uint32_t len);

/**
 * Find a character in the ringbuffer.
 *
 * @param ringbuffer
 * @param ch a character to find.
 * @return position where the character in, or -1 if not found.
 */
int ringbuffer_find(ringbuffer_t *ringbuffer, uint8_t ch);

#endif /* __RINGBUFFER_H */
