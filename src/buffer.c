#include "buffer.h"

void
FillRingBuffer(ringbuffer_t *pRingBuffer, uint8_t * pbuf, uint32_t len)
{
	uint32_t k;
	uint32_t w_ptr;

	w_ptr = pRingBuffer->w_ptr;
	for(k=0; k<len; k++)
	{
		pRingBuffer->buf[w_ptr] = pbuf[k];
		w_ptr = (w_ptr + 1) % pRingBuffer->length;
	}
	pRingBuffer->w_ptr = w_ptr;
}

void
ReadRingBuffer(ringbuffer_t *pRingBuffer, uint8_t *pbuf, uint32_t len)
{
	uint32_t k;
	uint32_t r_ptr;

	r_ptr = pRingBuffer->r_ptr;
	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[r_ptr];
		r_ptr = (r_ptr + 1) % pRingBuffer->length;
	}
	pRingBuffer->r_ptr = r_ptr;
}

void
LookRingBuffer(ringbuffer_t *pRingBuffer, uint8_t * pbuf, uint32_t len)
{
	uint32_t k;
	uint32_t r_ptr;

	r_ptr = pRingBuffer->r_ptr;
	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[r_ptr];
		r_ptr = (r_ptr + 1) % pRingBuffer->length;
	}
}

void
LookFromOffsetRingBuffer(ringbuffer_t *pRingBuffer, uint8_t * pbuf, uint32_t len, uint32_t offset)
{
	uint32_t k;
	uint32_t r_ptr;

	r_ptr = pRingBuffer->r_ptr;
	r_ptr = (r_ptr + offset) % pRingBuffer->length;               
	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[r_ptr];
		r_ptr = (r_ptr + 1) % pRingBuffer->length;
	}
}

uint32_t
GetDataLenInRingBuffer(ringbuffer_t *pRingBuffer)
{
	uint32_t len;
	uint32_t w_ptr = pRingBuffer->w_ptr;
	uint32_t r_ptr = pRingBuffer->r_ptr;
        
	if(r_ptr <= w_ptr)
	{
		len = w_ptr - r_ptr;
	}
	else
	{
		len = w_ptr - r_ptr + pRingBuffer->length;
	}

	return len;
}

uint32_t
GetFreeSpaceInRingBuffer(ringbuffer_t *pRingBuffer)
{
	return (pRingBuffer->length - GetDataLenInRingBuffer(pRingBuffer));
}

uint32_t
GetRingBufferSize(ringbuffer_t *pRingBuffer)
{
    return pRingBuffer->length;
}

void
FlushRingBuffer(ringbuffer_t *pRingBuffer, uint32_t len)
{       
	uint32_t r_ptr;

	r_ptr = pRingBuffer->r_ptr;
	r_ptr = (r_ptr + len) % pRingBuffer->length;
	pRingBuffer->r_ptr = r_ptr;
}

uint32_t
FindDataInRingBuffer(ringbuffer_t *pRingBuffer, uint32_t offset, uint8_t c)
{
	uint32_t pos = 0xFFFFFFFF;
	uint32_t len = GetDataLenInRingBuffer(pRingBuffer);
	uint32_t r_ptr;
	
	r_ptr = (pRingBuffer->r_ptr + offset) % pRingBuffer->length;
	
	for(int n=0; n<len; n++)
	{
			if( pRingBuffer->buf[r_ptr] == c )
			{
					pos = n;
					break;
			}
			r_ptr = (r_ptr + 1) % pRingBuffer->length;
	}
	
	return pos;
}

void
InitRingBuffer(ringbuffer_t *pRingBuffer, uint8_t * pbuf, uint32_t len)
{
	pRingBuffer->r_ptr = 0;
	pRingBuffer->w_ptr = 0;
	pRingBuffer->buf = pbuf;
	pRingBuffer->length = len;
}




