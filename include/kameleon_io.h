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

#ifndef __KAMELEON_IO_H
#define __KAMELEON_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "kameleon_list.h"

typedef struct kameleon_io_loop_s kameleon_io_loop_t;
typedef struct kameleon_io_handle_s kameleon_io_handle_t;
typedef struct kameleon_io_handle_list_s kameleon_io_handle_list_t;
typedef struct kameleon_io_timer_handle_s kameleon_io_timer_handle_t;
typedef struct kameleon_io_tty_handle_s kameleon_io_tty_handle_t;

/* callback function types */
typedef void (* kameleon_io_tty_read_cb)(unsigned int size, const char *buffer);
typedef void (* kameleon_io_timer_cb)();

/* handle types */
typedef enum kameleon_io_type {
  KAMELEON_IO_TIMER,
  KAMELEON_IO_TTY
} kameleon_io_type_t;

struct kameleon_io_handle_s {
  kameleon_list_node_t base;
  kameleon_io_type_t type;
  bool active;
};

struct kameleon_io_timer_handle_s {
  kameleon_io_handle_t base;
  kameleon_io_timer_cb timer_cb;
  uint64_t clamped_timeout;
  uint64_t interval;
  bool repeat;
};

struct kameleon_io_tty_handle_s {
  kameleon_io_handle_t base;
  kameleon_io_tty_read_cb read_cb;
};

/* loop type */
struct kameleon_io_loop_s {
  bool stop_flag;
  uint64_t time;
  kameleon_list_t timer_handles;
  kameleon_list_t tty_handles;
};

/* loop functions */
void kameleon_io_init();
void kameleon_io_run();

/* timer functions */
void kameleon_io_timer_init(kameleon_io_timer_handle_t *timer);
void kameleon_io_timer_close(kameleon_io_timer_handle_t *timer);
void kameleon_io_timer_start(kameleon_io_timer_handle_t *timer, kameleon_io_timer_cb timer_cb, uint64_t interval, bool repeat);
void kameleon_io_timer_stop(kameleon_io_timer_handle_t *timer);

/* TTY functions */
void kameleon_io_tty_init(kameleon_io_tty_handle_t *tty);
void kameleon_io_tty_read_start(kameleon_io_tty_handle_t *tty, kameleon_io_tty_read_cb read_cb);
void kameleon_io_tty_close(kameleon_io_tty_handle_t *tty);

#endif /* __KAMELEON_IO_H */