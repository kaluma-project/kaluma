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

#include "system.h"
#include "io.h"
#include "tty.h"

io_loop_t loop;

/* forward declarations */

static void io_timer_run();
static void io_tty_run();

/* general handle functions */

void io_handle_init(io_handle_t *handle, io_type_t type) {
  handle->type = type;
  handle->flags = 0;
  handle->close_cb = NULL;  
}

void io_handle_close(io_handle_t *handle, io_close_cb close_cb) {
  IO_SET_FLAG_ON(handle->flags, IO_FLAG_CLOSING);
  handle->close_cb = close_cb;
  list_append(&loop.closing_handles, (list_node_t *) handle);
}

static void io_update_time() {
  loop.time = gettime();
}

static void io_handle_closing() {
  while (loop.closing_handles.head != NULL) {
    io_handle_t *handle = (io_handle_t *) loop.closing_handles.head;
    list_remove(&loop.closing_handles, (list_node_t *) handle);
    if (handle->close_cb) {
      handle->close_cb(handle);
    }
  }
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
    io_handle_closing();
  }
}

/* timer functions */

uint32_t timer_count = 0;

void io_timer_init(io_timer_handle_t *timer) {
  io_handle_init((io_handle_t *) timer, IO_TIMER);
  timer->timer_id = timer_count++;
  timer->timer_cb = NULL;
}

void io_timer_start(io_timer_handle_t *timer, io_timer_cb timer_cb, uint64_t interval, bool repeat) {
  IO_SET_FLAG_ON(timer->base.flags, IO_FLAG_ACTIVE);
  timer->timer_cb = timer_cb;
  timer->clamped_timeout = loop.time + interval;
  timer->interval = interval;
  timer->repeat = repeat;
  list_append(&loop.timer_handles, (list_node_t *) timer);
}

void io_timer_stop(io_timer_handle_t *timer) {
  IO_SET_FLAG_OFF(timer->base.flags, IO_FLAG_ACTIVE);
  list_remove(&loop.timer_handles, (list_node_t *) timer);
}

io_timer_handle_t *io_timer_get_by_id(int timer_id) {
  io_timer_handle_t *handle = (io_timer_handle_t *) loop.timer_handles.head;
  while (handle != NULL) {
    if (handle->timer_id == timer_id) {
      return handle;
    }
    handle = (io_timer_handle_t *) ((list_node_t *) handle)->next;
  }
  return NULL;
}

static void io_timer_run() {
  io_timer_handle_t *handle = (io_timer_handle_t *) loop.timer_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      if (handle->clamped_timeout < loop.time) {
        if (handle->repeat) {
          handle->clamped_timeout = handle->clamped_timeout + handle->interval;
        } else {
          IO_SET_FLAG_OFF(handle->base.flags, IO_FLAG_ACTIVE);
        }
        if (handle->timer_cb) {
          handle->timer_cb(handle);
        }
      }
    }
    handle = (io_timer_handle_t *) ((list_node_t *) handle)->next;
  }
}

/* TTY functions */

void io_tty_init(io_tty_handle_t *tty) {
  io_handle_init((io_handle_t *) tty, IO_TTY);
  tty->read_cb = NULL;
}

void io_tty_read_start(io_tty_handle_t *tty, io_tty_read_cb read_cb) {
  IO_SET_FLAG_ON(tty->base.flags, IO_FLAG_ACTIVE);
  tty->read_cb = read_cb;
  list_append(&loop.tty_handles, (list_node_t *) tty);
}

void io_tty_read_stop(io_tty_handle_t *tty) {
  IO_SET_FLAG_OFF(tty->base.flags, IO_FLAG_ACTIVE);
  list_remove(&loop.tty_handles, (list_node_t *) tty);
}

static void io_tty_run() {
  io_tty_handle_t *handle = (io_tty_handle_t *) loop.tty_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      if (handle->read_cb != NULL && tty_has_data()) {
        unsigned int size = tty_data_size();
        for (int i = 0; i < size; i++) {
          handle->read_cb(tty_getc());
        }
      }
    }
    handle = (io_tty_handle_t *) ((list_node_t *) handle)->next;
  }
}
