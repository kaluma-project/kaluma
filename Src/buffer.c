#include "buffer.h"

void
FillRingBuffer(RINGBUFFER *pRingBuffer, unsigned char * pbuf, unsigned int len)
{
	unsigned int k;

	for(k=0; k<len; k++)
	{
		pRingBuffer->buf[pRingBuffer->w_ptr] = pbuf[k];
		pRingBuffer->w_ptr = pRingBuffer->w_ptr + 1;
		pRingBuffer->w_ptr = pRingBuffer->w_ptr % BUFFER_LEN;
	}
}

void
ReadRingBuffer(RINGBUFFER *pRingBuffer, unsigned char *pbuf, unsigned int len)
{
	unsigned int k;

	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[pRingBuffer->r_ptr];
		pRingBuffer->r_ptr = pRingBuffer->r_ptr + 1;
		pRingBuffer->r_ptr = pRingBuffer->r_ptr % BUFFER_LEN;
	}
}

void
LookRingBuffer(RINGBUFFER *pRingBuffer, unsigned char * pbuf, unsigned int len)
{
	unsigned int k;
	unsigned int ptr;

	ptr = pRingBuffer->r_ptr;

	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[pRingBuffer->r_ptr];
		pRingBuffer->r_ptr = pRingBuffer->r_ptr + 1;
		pRingBuffer->r_ptr = pRingBuffer->r_ptr % BUFFER_LEN;
	}
	pRingBuffer->r_ptr = ptr;
}

void
LookFromOffsetRingBuffer(RINGBUFFER *pRingBuffer, unsigned char * pbuf, unsigned int len, unsigned int offset)
{
	unsigned int k;
	unsigned int ptr;

	ptr = pRingBuffer->r_ptr;

        pRingBuffer->r_ptr = (pRingBuffer->r_ptr + offset) % BUFFER_LEN;               
	for(k=0; k<len; k++)
	{
		pbuf[k] = pRingBuffer->buf[pRingBuffer->r_ptr];
		pRingBuffer->r_ptr = pRingBuffer->r_ptr + 1;
		pRingBuffer->r_ptr = pRingBuffer->r_ptr % BUFFER_LEN;
	}
	pRingBuffer->r_ptr = ptr;
}

unsigned int
GetDataLenInRingBuffer(RINGBUFFER *pRingBuffer)
{
	unsigned int len;

	if(pRingBuffer->r_ptr <= pRingBuffer->w_ptr)
	{
		len = pRingBuffer->w_ptr - pRingBuffer->r_ptr;
	}
	else
	{
		len = pRingBuffer->w_ptr - pRingBuffer->r_ptr + BUFFER_LEN;
	}

	return len;
}

void
InitRingBuffer(RINGBUFFER *pRingBuffer)
{
	pRingBuffer->r_ptr = 0;
	pRingBuffer->w_ptr = 0;
}

