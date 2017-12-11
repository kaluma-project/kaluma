
#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdint.h>

//
//  C-Implementation on RING BUFFER
//
typedef struct {
	uint8_t * buf;
  uint32_t	length;
	uint32_t	r_ptr;
	uint32_t  w_ptr;
} ringbuffer_t;

uint32_t	GetDataLenInRingBuffer(ringbuffer_t *);
uint32_t    GetFreeSpaceInRingBuffer(ringbuffer_t *pRingBuffer);
void		ReadRingBuffer(ringbuffer_t *, uint8_t *, uint32_t);
void		FillRingBuffer(ringbuffer_t *, uint8_t *, uint32_t);
void		LookRingBuffer(ringbuffer_t *, uint8_t *, uint32_t);
void            FlushRingBuffer(ringbuffer_t *pRingBuffer, uint32_t);
void            LookFromOffsetRingBuffer(ringbuffer_t *pRingBuffer, uint8_t * pbuf, uint32_t len, uint32_t offset);
void            InitRingBuffer(ringbuffer_t *, uint8_t *, uint32_t);
uint32_t    FindDataInRingBuffer(ringbuffer_t *pRingBuffer, uint32_t offset, uint8_t c);
uint32_t    GetRingBufferSize(ringbuffer_t *pRingBuffer);

#endif