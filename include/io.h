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

#ifndef __IO_H
#define __IO_H

#include <stdbool.h>

typedef struct io_loop_s io_loop_t;
typedef struct io_handle_s io_handle_t;
typedef struct io_handle_list_s io_handle_list_t;
typedef struct io_idle_handle_s io_idle_handle_t;
typedef struct io_timer_handle_s io_timer_handle_t;
typedef struct io_tty_handle_s io_tty_handle_t;

/* callback function types */
typedef void (* io_tty_read_cb)(unsigned int size, const char *buffer);
typedef void (* io_timer_cb)();
typedef void (* io_idler_cb)();

/* handle types */
typedef enum io_type {
  IO_IDLE,
  IO_TIMER,
  IO_TTY
} io_type_t;

struct io_handle_s {
  io_type_t type;
  io_handle_t *prev; 
  io_handle_t *next;
};

struct io_handle_list_s {
  io_handle_t *head;
  io_handle_t *tail;
};

struct io_idle_handle_s {
  io_handle_t base;
  io_idler_cb idler_cb;
};

struct io_timer_handle_s {
  io_handle_t base;
  io_timer_cb timer_cb;
};

struct io_tty_handle_s {
  io_handle_t base;
  io_tty_read_cb read_cb;
};

/* loop type */
struct io_loop_s {
  bool stop_flag;
  io_handle_list_t timer_handles;
  io_handle_list_t tty_handles;
  io_handle_list_t idle_handles;
};

/* loop functions */
void io_init();
void io_run();

/* idle functions */
void io_idle_init(io_idle_handle_t *idle, io_idler_cb idler);
void io_idle_close(io_idle_handle_t *idle);

/* timer functions */
void io_timer_init(io_timer_handle_t *timer);
void io_timer_close(io_timer_handle_t *timer);
void io_timer_start(io_timer_handle_t *timer, io_timer_cb timer_cb, uint64_t timeout, bool repeat);
void io_timer_stop(io_timer_handle_t *timer);

/* TTY functions */
void io_tty_init(io_tty_handle_t *tty);
void io_tty_read_start(io_tty_handle_t *tty, io_tty_read_cb read_cb);
void io_tty_close(io_tty_handle_t *tty);

#endif /* __IO_H */