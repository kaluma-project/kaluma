
#ifndef __BUFFER_H__
#define __BUFFER_H__

#if !defined(BYTE) && !defined(DWORD)
typedef unsigned char BYTE;
typedef unsigned int DWORD;
#endif

#define		BUFFER_LEN	(1024*1)

//
//  C-Implementation on RING BUFFER
//
typedef struct
{
	BYTE	buf[BUFFER_LEN];
	DWORD	r_ptr;
	DWORD	w_ptr;
} RINGBUFFER;

unsigned int	GetDataLenInRingBuffer(RINGBUFFER *);
void		ReadRingBuffer(RINGBUFFER *, unsigned char *, unsigned int);
void		FillRingBuffer(RINGBUFFER *, unsigned char *, unsigned int);
void		LookRingBuffer(RINGBUFFER *, unsigned char *, unsigned int);
void            LookFromOffsetRingBuffer(RINGBUFFER *pRingBuffer, unsigned char * pbuf, unsigned int len, unsigned int offset);
void		InitRingBuffer(RINGBUFFER *);


#endif
