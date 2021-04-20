/* Copyright (c) 2017 Kaluma
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

#ifndef ___KM_IO_H
#define ___KM_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "utils.h"
#include "jerryscript.h"

typedef struct km_io_loop_s km_io_loop_t;
typedef struct km_io_handle_s km_io_handle_t;
typedef struct km_io_handle_list_s km_io_handle_list_t;
typedef struct km_io_timer_handle_s km_io_timer_handle_t;
typedef struct km_io_tty_handle_s km_io_tty_handle_t;
typedef struct km_io_watch_handle_s km_io_watch_handle_t;
typedef struct km_io_uart_handle_s km_io_uart_handle_t;
typedef struct km_io_idle_handle_s km_io_idle_handle_t;

/* handle flags */

#define KM_IO_FLAG_ACTIVE 0x01
#define KM_IO_FLAG_CLOSING 0x02

#define KM_IO_SET_FLAG_ON(field, flag) ((field) |= (flag))
#define KM_IO_SET_FLAG_OFF(field, flag) ((field) &= ~(flag))
#define KM_IO_HAS_FLAG(field, flag) ((field) & (flag))

/* general handle types */

typedef enum km_io_type
{
  KM_IO_TIMER,
  KM_IO_TTY,
  KM_IO_WATCH,
  KM_IO_UART,
  KM_IO_IDLE
} km_io_type_t;

typedef void (*km_io_close_cb)(km_io_handle_t *);

struct km_io_handle_s
{
  km_list_node_t base;
  uint32_t id;
  km_io_type_t type;
  uint8_t flags;
  km_io_close_cb close_cb;
};

/* timer handle types */

typedef void (*km_io_timer_cb)(km_io_timer_handle_t *);

struct km_io_timer_handle_s
{
  km_io_handle_t base;
  km_io_timer_cb timer_cb;
  jerry_value_t timer_js_cb;
  uint64_t clamped_timeout;
  uint64_t interval;
  bool repeat;
  uint32_t tag; // for application use
};

/* TTY handle types */

typedef void (*km_io_tty_read_cb)(uint8_t *, size_t);

struct km_io_tty_handle_s
{
  km_io_handle_t base;
  km_io_tty_read_cb read_cb;
};

/* GPIO watch handle types */

typedef enum
{
  KM_IO_WATCH_MODE_FALLING,
  KM_IO_WATCH_MODE_RISING,
  KM_IO_WATCH_MODE_CHANGE,
} km_io_watch_mode_t;

typedef void (*km_io_watch_cb)(km_io_watch_handle_t *);

struct km_io_watch_handle_s
{
  km_io_handle_t base;
  km_io_watch_mode_t mode;
  uint8_t pin;
  uint64_t debounce_time;
  uint32_t debounce_delay;
  uint8_t last_val;
  uint8_t val;
  km_io_watch_cb watch_cb;
  jerry_value_t watch_js_cb;
};

/* UART handle type */

typedef int (*km_io_uart_available_cb)(km_io_uart_handle_t *);
typedef void (*km_io_uart_read_cb)(km_io_uart_handle_t *, uint8_t *, size_t);

struct km_io_uart_handle_s
{
  km_io_handle_t base;
  uint8_t port;
  km_io_uart_available_cb available_cb;
  km_io_uart_read_cb read_cb;
  jerry_value_t read_js_cb;
};

/* idle handle types */

typedef void (*km_io_idle_cb)(km_io_idle_handle_t *);

struct km_io_idle_handle_s
{
  km_io_handle_t base;
  km_io_idle_cb idle_cb;
};

/* loop type */

struct km_io_loop_s
{
  bool stop_flag;
  uint64_t time;
  km_list_t timer_handles;
  km_list_t tty_handles;
  km_list_t watch_handles;
  km_list_t uart_handles;
  km_list_t idle_handles;
  km_list_t closing_handles;
};

/* loop functions */

void km_io_init();
void km_io_run();

/* general handle functions */

void km_io_handle_init(km_io_handle_t *handle, km_io_type_t type);
void km_io_handle_close(km_io_handle_t *handle, km_io_close_cb close_cb);
km_io_handle_t *km_io_handle_get_by_id(uint32_t id, km_list_t *handle_list);

/* timer functions */

void km_io_timer_init(km_io_timer_handle_t *timer);
void km_io_timer_start(km_io_timer_handle_t *timer, km_io_timer_cb timer_cb, uint64_t interval, bool repeat);
void km_io_timer_stop(km_io_timer_handle_t *timer);
km_io_timer_handle_t *km_io_timer_get_by_id(uint32_t id);
void km_io_timer_cleanup();

/* TTY functions */

void km_io_tty_init(km_io_tty_handle_t *tty);
void km_io_tty_read_start(km_io_tty_handle_t *tty, km_io_tty_read_cb read_cb);
void km_io_tty_read_stop(km_io_tty_handle_t *tty);
void km_io_tty_cleanup();

/* GPIO watch functions */

void km_io_watch_init(km_io_watch_handle_t *watch);
void km_io_watch_start(km_io_watch_handle_t *watch, km_io_watch_cb watch_cb, uint8_t pin, km_io_watch_mode_t mode, uint32_t debounce);
void km_io_watch_stop(km_io_watch_handle_t *watch);
km_io_watch_handle_t *km_io_watch_get_by_id(uint32_t id);
void km_io_watch_cleanup();

/* UART function */

void km_io_uart_init(km_io_uart_handle_t *uart);
void km_io_uart_read_start(km_io_uart_handle_t *uart, uint8_t port, km_io_uart_available_cb available_cb, km_io_uart_read_cb read_cb);
void km_io_uart_read_stop(km_io_uart_handle_t *uart);
km_io_uart_handle_t *km_io_uart_get_by_id(uint32_t id);
void km_io_uart_cleanup();

/* idle function */

void km_io_idle_init(km_io_idle_handle_t *idle);
void km_io_idle_start(km_io_idle_handle_t *idle, km_io_idle_cb idle_cb);
void km_io_idle_stop(km_io_idle_handle_t *idle);
km_io_idle_handle_t *km_io_idle_get_by_id(uint32_t id);
void km_io_idle_cleanup();

#endif /* ___KM_IO_H */
