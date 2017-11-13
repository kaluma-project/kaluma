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

static void __io_idle_run();
static void __io_timer_run();
static void __io_tty_run();

/* loop functions */

void io_init() {
  loop.stop_flag = false;
  loop.tty_handles = NULL;
}

void io_run() {
  while (1) {
    __io_tty_run();
  }
}

/* idle functions */

void io_idle_init(io_idle_handle_t *idle, io_idler_cb idler) {
  idle->type = IO_IDLE;
  idle->idler_cb = idler;
  // add to loop.idle_handles
}

void io_idle_close(io_idle_handle_t *idle) {

}

static void __io_idle_run() {

}

/* timer functions */

void io_timer_init(io_timer_handle_t *timer) {
  timer->type = IO_TIMER;
  // add to loop.timer_handles
}

void io_timer_close(io_timer_handle_t *timer) {

}

void io_timer_start(io_timer_handle_t *timer, io_timer_cb timer_cb, uint64_t timeout, bool repeat) {

}

void io_timer_stop(io_timer_handle_t *timer) {

}

static void __io_timer_run() {

}

/* handle functions */

static void __io_list_init(io_handle_list_t *list) {
  list->head = NULL;
  list->tail = NULL;
}

static void __io_list_push(io_handle_list_t *list, io_handle_t *handle) {
  if (list->tail == NULL && list->head == NULL) {
    list->head = handle;
    list->tail = handle;
    handle->next = NULL;
    handle->prev = NULL;
  } else {
    list->tail->next = handle;
    handle->prev = list->tail;
    handle->next = NULL;
  }
}

static void __io_list_remove(io_handle_list_t *list, io_handle_t *handle) {
  if (list->head == handle) {
    list->head = handle->next;
  }
  if (list->tail == handle) {
    list->tail = handle->prev;
  }
}

/* TTY functions */

void io_tty_init(io_tty_handle_t *tty) {
  tty->base.type = IO_TTY;
  tty->read_cb = NULL;
  loop.tty_handles = tty;
  __io_add(loop.tty_handles, (io_handle_t *) tty);
}

void io_tty_read_start(io_tty_handle_t *tty, io_tty_read_cb read_cb) {
  tty->read_cb = read_cb;
}

void io_tty_close(io_tty_handle_t *tty) {
  io_tty_handle_t *p = loop.tty_handles;
  if (p == NULL) {
    /* already empty */
  } else if (p == tty) {
    loop.tty_handles = p->next;
  } else {
    while (p->next != NULL) {
      if (p->next == tty) {
        p->next = tty->next;
        break;
      }
      p = p->next;
    }
  }
}

static void __io_tty_run() {
  io_tty_handle_t *p = loop.tty_handles;
  while (p != NULL) {
    if (p->read_cb != NULL && tty_has_data()) {
      p->read_cb(tty_read_size(), tty_read());
    }
    p = p->next;
  }
}