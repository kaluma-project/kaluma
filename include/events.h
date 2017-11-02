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

#ifndef __EVENTS_H
#define __EVENTS_H

typedef struct {
	int stop_flag;
	void *tty_handles;
} ev_loop_t;

typedef enum {
	EV_TTY,
	EV_IDLE,
	EV_TIMER
} ev_handle_type_t;

typedef void (*ev_tty_read_cb)(ev_tty_t* tty, unsigned int nread, const char* buf);

typedef struct {
	ev_handle_type_t handle_type;
	ev_tty_t *next;
	ev_tty_t *prev;
	ev_tty_read_cb read_cb;
} ev_tty_t;

#define QUEUE_PUSH(queue, item)       \
	do {                                \
		if ((queue) == NULL) {            \
			(queue) = (item);               \
		} else {                          \
			(item)->next = (queue);         \
			(item)->next->prev = (item);    \
			(item)->prev = (queue)->prev;   \
			(queue)->prev->next = (item);   \
			(queue) = (item)                \
		}                                 \
	} while (0)                         \

/*
typedef struct {
	EV_HANDLE_FIELDS
	ev_idle_cb idle_cb;
} ev_idle_t;

typedef struct {
	EV_HANDLE_FIELDS
	ev_io_poll_cb poll_cb;
} ev_stream_t;

typedef struct {
	EV_HANDLE_FIELDS
	int timeout;
	int repeat;
} ev_timer_t;
*/

void ev_init();
void ev_run();

void ev_tty_init(ev_tty_t *tty);
void ev_tty_start(ev_tty_t *tty, ev_tty_read_cb read_cb);
void ev_tty_close(ev_tty_t *tty);

/*
void ev_idle_init(ev_idle_t *idler);
void ev_idle_start(ev_idle_t *idler);
void ev_idle_close(ev_idle_t *idler);
void ev_run_idle(ev_loop_t *loop);
*/

#endif /* __EVENTS_H */
