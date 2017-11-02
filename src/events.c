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

#include <stdio.h>
#include "events.h"
#include "tty.h"

/* A global default loop */
ev_loop_t loop;


void ev_init() {
  loop.stop_flag = 0;
  loop.tty_handles = NULL;
}


void ev_run() {
  while (loop.stop_flag == 0) {
    ev_run_tty();
  }
}


void ev_tty_init(ev_tty_t *tty) {
  tty->handle_type = EV_TTY;
  tty_init();
  QUEUE_PUSH(loop.tty_handles,tty);

}

void ev_tty_start(ev_tty_t *tty, ev_tty_read_cb read_cb) {
  tty->read_cb = read_cb;
  usb_data_receive(tty->read_cb);
}

void ev_tty_close(ev_tty_t *tty) {
  tty->read_cb = NULL;
  usb_data_receive(tty->read_cb);
}

void ev_tty_run() {

}