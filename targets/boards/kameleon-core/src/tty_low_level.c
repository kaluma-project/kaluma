/* Copyright (c) 2017 Kameleon
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
 */
#include "buffer.h"
#include "usbd_cdc_if.h"

#define TTY_TX_RINGBUFFER_SIZE 1024
#define TTY_RX_RINGBUFFER_SIZE 1024

static unsigned char tty_tx_buffer[TTY_TX_RINGBUFFER_SIZE];
static unsigned char tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t tty_tx_ringbuffer;
static ringbuffer_t tty_rx_ringbuffer;

void tty_init_ringbuffer() {
	InitRingBuffer(&tty_tx_ringbuffer, tty_tx_buffer, sizeof(tty_tx_buffer));
	InitRingBuffer(&tty_rx_ringbuffer, tty_rx_buffer, sizeof(tty_rx_buffer));
}

/* this function is called in the pendable interrupt service routine which has lowest priority
   to allow other interrupts service.
*/
void tty_transmit_data() {   
	/* if the previous data is under transmitting, just return to avoid blocking */
	if (CDC_Transmit_IsReady()) {
		uint32_t len = GetDataLenInRingBuffer(&tty_tx_ringbuffer);
		if (len) {
			uint8_t buf[TTY_TX_RINGBUFFER_SIZE];
			ReadRingBuffer(&tty_tx_ringbuffer, buf, len);      
			CDC_Transmit_FS(buf, len);
		}
	}
}

uint32_t tty_get_tx_freespace() {
	return GetFreeSpaceInRingBuffer(&tty_tx_ringbuffer);
}

uint32_t tty_get_rx_freespace() {
	return GetFreeSpaceInRingBuffer(&tty_rx_ringbuffer);
}

uint32_t tty_get_rx_data_length() {
	return GetDataLenInRingBuffer(&tty_rx_ringbuffer);
}

uint32_t tty_get_tx_data_length() {
	return GetDataLenInRingBuffer(&tty_tx_ringbuffer);
}

uint8_t tty_get_byte() {
	uint8_t c;
	ReadRingBuffer(&tty_rx_ringbuffer, &c, 1);
	return c;
}

uint32_t tty_get_bytes(uint8_t * buf, uint32_t nToRead) {
	/* interrupt level masking */
	uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
	uint32_t pre_emption, sub_priority;
	HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
	__set_BASEPRI(pre_emption << 4);
				
	if (tty_get_rx_data_length() < nToRead) {
		nToRead = tty_get_rx_data_length();
	}

	__set_BASEPRI(0);
	ReadRingBuffer(&tty_rx_ringbuffer, buf, nToRead);
	return nToRead;
}

uint32_t tty_fill_rx_bytes(uint8_t * buf, uint32_t nToWrite) {
	/* interrupt level masking */
	uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
	uint32_t pre_emption, sub_priority;
	HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
	__set_BASEPRI(pre_emption << 4);

	if (tty_get_rx_freespace() < nToWrite) {
		nToWrite = tty_get_rx_freespace();
	}
	__set_BASEPRI(0);
	
	FillRingBuffer(&tty_rx_ringbuffer, buf, nToWrite);
	return nToWrite;
}

void tty_put_byte(uint8_t c) {
	FillRingBuffer(&tty_tx_ringbuffer, (uint8_t *)&c, 1);
}

uint32_t tty_put_bytes(uint8_t * buf, uint32_t nToWrite) {
	/* interrupt level masking */
	uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
	uint32_t pre_emption, sub_priority;
	HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
	__set_BASEPRI(pre_emption << 4);

	if (tty_get_tx_freespace() < nToWrite) {
		nToWrite = tty_get_tx_freespace();
	}
	__set_BASEPRI(0);

	FillRingBuffer(&tty_tx_ringbuffer, buf, nToWrite);
	return nToWrite;
}

