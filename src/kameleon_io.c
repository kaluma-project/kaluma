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
#include "kameleon_io.h"
#include "kameleon_tty.h"

kameleon_io_loop_t loop;

/* forward declarations */

static void __io_idle_run();
static void __io_timer_run();
static void __io_tty_run();

/* loop functions */

void kameleon_io_init() {
  loop.stop_flag = false;
  kameleon_list_init(&loop.tty_handles);
}

void kameleon_io_run() {
  while (1) {
    __io_tty_run();
  }
}

/* idle functions */

void kameleon_io_idle_init(kameleon_io_idle_handle_t *idle, kameleon_io_idler_cb idler) {
  idle->base.type = KAMELEON_IO_IDLE;
  idle->idler_cb = idler;
  // add to loop.idle_handles
}

void kameleon_io_idle_close(kameleon_io_idle_handle_t *idle) {

}

static void __io_idle_run() {

}

/* timer functions */

void kameleon_io_timer_init(kameleon_io_timer_handle_t *timer) {
  timer->base.type = KAMELEON_IO_TIMER;
  timer->timer_cb = NULL;
  kameleon_list_append(&loop.timer_handles, timer);
}

void kameleon_io_timer_close(kameleon_io_timer_handle_t *timer) {

}

void kameleon_io_timer_start(kameleon_io_timer_handle_t *timer, kameleon_io_timer_cb timer_cb, uint64_t timeout, bool repeat) {

}

void kameleon_io_timer_stop(kameleon_io_timer_handle_t *timer) {

}

static void __io_timer_run() {

}

/* TTY functions */

void kameleon_io_tty_init(kameleon_io_tty_handle_t *tty) {
  tty->base.type = KAMELEON_IO_TTY;
  tty->read_cb = NULL;
  kameleon_list_append(&loop.tty_handles, tty);
}

void kameleon_io_tty_read_start(kameleon_io_tty_handle_t *tty, kameleon_io_tty_read_cb read_cb) {
  tty->read_cb = read_cb;
}

void kameleon_io_tty_close(kameleon_io_tty_handle_t *tty) {
  kameleon_io_tty_handle_t *p = &loop.tty_handles;
}

static void __io_tty_run() {
  kameleon_io_tty_handle_t *p = &loop.tty_handles;
  while (p != NULL) {
    if (p->read_cb != NULL && kameleon_tty_has_data()) {
      p->read_cb(kameleon_tty_read_size(), kameleon_tty_read());
    }
    p = ((kameleon_list_node_t *) p)->next;
  }
}
