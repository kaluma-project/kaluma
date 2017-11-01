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

typedef enum {
	EV_TTY,
	EV_IDLE,
	EV_TIMER
} ev_handle_type_t;

typedef struct {
	void *tty_handles;
	void *idle_handles;
	void *timer_handles;
} ev_loop_t;

typedef struct {
	ev_loop_t *loop;
	ev_handle_type_t handle_type;
	void *next;
	void *prev;
} ev_handle_t;

typedef void (ev_tty_read_cb)(ev_tty_t* tty, ssize_t nread, const char* buf);

typedef struct {
	// inhert from handle fields...
	ev_tty_read_cb tty_read_cb;
} ev_tty_t;

typedef struct {
	// inhert from handle fields...
	// idle_cb_t idle_cb;
} ev_idle_t;

typedef struct {
	// inhert from handle fields...
	int timeout;
	int repeat;
} ev_timer_t;

ev_loop_t default_loop;

ev_loop_t *ev_default_loop() {
	return &default_loop;
}

void ev_tty_init(ev_tty_t *tty) {
	tty->loop = loop;
	tty->handle_type = EV_TTY;
	QUEUE_PUSH(loop->tty_handles, tty);
	// ...
}

void ev_tty_start(ev_tty_t *tty, ev_tty_read_cb read_cb) {
	tty->read_cb = read_cb;
	usb_data_receive(tty->read_cb);
}

void ev_tty_close(ev_tty_t *tty) {
	tty->read_cb = NULL;
	usb_data_receive(tty->read_cb);
}

void ev_idle_init(ev_loop_t *loop, ev_idle_t *idler);
void ev_idle_start(ev_idle_t *idler);
void ev_idle_close(ev_idle_t *idler);
void ev_run_idle(ev_loop_t *loop);

void ev_timer_init(ev_loop_t *loop, ev_timer_t *timer);
void ev_timer_start(ev_timer_t *timer);
void ev_timer_close(ev_timer_t *timer);
void ev_run_timer(ev_loop_t *loop);

#endif /* __EVENTS_H */
