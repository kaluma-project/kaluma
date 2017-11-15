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

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "io.h"
#include "tty.h"

io_loop_t loop;

/* forward declarations */

static void io_timer_run();
static void io_tty_run();

/* internal functions */

static void io_update_time() {
  loop.time = gettime();
}

/* loop functions */

void io_init() {
  loop.stop_flag = false;
  list_init(&loop.tty_handles);
}

void io_run() {
  while (1) {
    io_update_time();
    io_timer_run();
    io_tty_run();
  }
}

/* timer functions */

void io_timer_init(io_timer_handle_t *timer) {
  timer->base.type = IO_TIMER;
  timer->base.active = false;
  timer->timer_cb = NULL;
  list_append(&loop.timer_handles, timer);
}

void io_timer_close(io_timer_handle_t *timer) {
  list_remove(&loop.timer_handles, timer);
}

void io_timer_start(io_timer_handle_t *timer, io_timer_cb timer_cb, uint64_t interval, bool repeat) {
  timer->base.active = true;
  timer->timer_cb = timer_cb;
  timer->clamped_timeout = loop.time + interval;
  timer->interval = interval;
  timer->repeat = repeat;
}

void io_timer_stop(io_timer_handle_t *timer) {
  timer->base.active = false;
}

static void io_timer_run() {
  io_timer_handle_t *handle = &loop.timer_handles;
  while (handle != NULL) {
    if (handle->base.active) {
      if (handle->clamped_timeout < loop.time) {
        if (handle->repeat) {
          handle->clamped_timeout = handle->clamped_timeout + handle->interval;
        }
        if (handle->timer_cb != NULL) {
          handle->timer_cb();
        }
      }
    }
    handle = ((list_node_t *) handle)->next;
  }
}

/* TTY functions */

void io_tty_init(io_tty_handle_t *tty) {
  tty->base.type = IO_TTY;
  tty->read_cb = NULL;
  list_append(&loop.tty_handles, tty);
}

void io_tty_close(io_tty_handle_t *tty) {
  list_remove(&loop.tty_handles, tty);
}

void io_tty_read_start(io_tty_handle_t *tty, io_tty_read_cb read_cb) {
  tty->base.active = true;
  tty->read_cb = read_cb;
}

void io_tty_read_stop(io_tty_handle_t *tty) {
  tty->base.active = false;
}

static void io_tty_run() {
  io_tty_handle_t *handle = &loop.tty_handles;
  while (handle != NULL) {
    if (handle->base.active) {
      if (handle->read_cb != NULL && tty_has_data()) {
        unsigned int size = tty_data_size();
        for (int i = 0; i < size; i++) {
          handle->read_cb(tty_getc());
        }
      }
    }
    handle = ((list_node_t *) handle)->next;
  }
}
