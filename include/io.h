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

#include <stdbool.h>
#include <stdint.h>

#include "jerryscript.h"
#include "utils.h"

typedef struct km_io_loop_s km_io_loop_t;
typedef struct km_io_handle_s km_io_handle_t;
typedef struct km_io_handle_list_s km_io_handle_list_t;
typedef struct km_io_stream_handle_s km_io_stream_handle_t;
typedef struct km_io_timer_handle_s km_io_timer_handle_t;
typedef struct km_io_tty_handle_s km_io_tty_handle_t;
typedef struct km_io_watch_handle_s km_io_watch_handle_t;
typedef struct km_io_uart_handle_s km_io_uart_handle_t;
typedef struct km_io_idle_handle_s km_io_idle_handle_t;
typedef struct km_io_tcp_handle_s km_io_tcp_handle_t;

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
  KM_IO_IDLE,
  KM_IO_STREAM,
  KM_IO_SOCKET
} km_io_type_t;

/* base handle type */

typedef void (*km_io_close_cb)(km_io_handle_t *);

#define KM_IO_HANDLE_FIELDS \
  km_list_node_t base;      \
  uint32_t id;              \
  km_io_type_t type;        \
  uint8_t flags;            \
  km_io_close_cb close_cb;

struct km_io_handle_s
{
  KM_IO_HANDLE_FIELDS
};

/* stream handle type */

typedef int (*km_io_stream_available_cb)(km_io_stream_handle_t *);
typedef void (*km_io_stream_read_cb)(km_io_stream_handle_t *, uint8_t *,
                                     size_t);

#define KM_IO_STREAM_FIELDS               \
  bool blocking;                          \
  km_io_stream_available_cb available_cb; \
  km_io_stream_read_cb read_cb;

struct km_io_stream_handle_s
{
  KM_IO_HANDLE_FIELDS
  KM_IO_STREAM_FIELDS
};

/* timer handle types */

typedef void (*km_io_timer_cb)(km_io_timer_handle_t *);

struct km_io_timer_handle_s
{
  KM_IO_HANDLE_FIELDS
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
  KM_IO_HANDLE_FIELDS
  km_io_tty_read_cb read_cb;
};

/* GPIO watch handle types */

typedef enum
{
  KM_IO_WATCH_MODE_LOW_LEVEL = 1,  // BIT0
  KM_IO_WATCH_MODE_HIGH_LEVEL = 2, // BIT1
  KM_IO_WATCH_MODE_FALLING = 4,    // BIT2
  KM_IO_WATCH_MODE_RISING = 8,     // BIT3
  KM_IO_WATCH_MODE_CHANGE = 12,    // BIT2 | BIT3
} km_io_watch_mode_t;

typedef void (*km_io_watch_cb)(km_io_watch_handle_t *);

struct km_io_watch_handle_s
{
  KM_IO_HANDLE_FIELDS
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
  KM_IO_HANDLE_FIELDS
  uint8_t port;
  km_io_uart_available_cb available_cb;
  km_io_uart_read_cb read_cb;
  jerry_value_t read_js_cb;
};

/* idle handle types */

typedef void (*km_io_idle_cb)(km_io_idle_handle_t *);

struct km_io_idle_handle_s
{
  KM_IO_HANDLE_FIELDS
  km_io_idle_cb idle_cb;
};

/* tcp handle types */

typedef void (*km_io_tcp_cb)(km_io_tcp_handle_t *);
typedef void (*km_io_tcp_connect_cb)(km_io_tcp_handle_t *, int);
typedef void (*km_io_tcp_accept_cb)(km_io_tcp_handle_t *);
typedef int (*km_io_tcp_available_cb)(km_io_tcp_handle_t *);
typedef void (*km_io_tcp_read_cb)(km_io_tcp_handle_t *, uint8_t *,
                                  size_t);

#define KM_IO_TCP_FIELDS                   \
  int fd;                                  \
  km_io_tcp_connect_cb tcp_connect_cb;     \
  km_io_tcp_accept_cb tcp_accept_cb;       \
  km_io_tcp_available_cb tcp_available_cb; \
  /* km_io_tcp_write_cb tcp_write_cb */    \
  km_io_tcp_read_cb tcp_read_cb;           \
  /* km_io_tcp_error_cb tcp_error_cb */    \
  km_io_tcp_cb tcp_close_cb;

struct km_io_tcp_handle_s
{
  KM_IO_HANDLE_FIELDS
  KM_IO_STREAM_FIELDS
  KM_IO_TCP_FIELDS
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
  km_list_t stream_handles;
  km_list_t socket_handles;
  km_list_t closing_handles;
};

/* loop functions */

void km_io_init();
void km_io_cleanup();
void km_io_run(bool infinite);

/* general handle functions */

void km_io_handle_init(km_io_handle_t *handle, km_io_type_t type);
void km_io_handle_close(km_io_handle_t *handle, km_io_close_cb close_cb);
km_io_handle_t *km_io_handle_get_by_id(uint32_t id, km_list_t *handle_list);

/* stream functions */

void km_io_stream_init(km_io_stream_handle_t *stream);
// void km_io_stream_set_blocking(km_io_stream_handle_t *stream, bool blocking);
void km_io_stream_read_start(km_io_stream_handle_t *stream,
                             km_io_stream_available_cb available_cb,
                             km_io_stream_read_cb read_cb);
void km_io_stream_read_stop(km_io_stream_handle_t *stream);
void km_io_stream_cleanup();
// int km_io_stream_is_readable(km_io_stream_handle_t *stream);
// int km_io_stream_read(km_io_stream_handle_t *stream);
// void km_io_stream_push(km_io_stream_handle_t *stream, uint8_t *buffer, size_t
// size); // push to read buffer

/* timer functions */

void km_io_timer_init(km_io_timer_handle_t *timer);
void km_io_timer_start(km_io_timer_handle_t *timer, km_io_timer_cb timer_cb,
                       uint64_t interval, bool repeat);
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
void km_io_watch_start(km_io_watch_handle_t *watch, km_io_watch_cb watch_cb,
                       uint8_t pin, km_io_watch_mode_t mode, uint32_t debounce);
void km_io_watch_stop(km_io_watch_handle_t *watch);
km_io_watch_handle_t *km_io_watch_get_by_id(uint32_t id);
void km_io_watch_cleanup();

/* UART function */

void km_io_uart_init(km_io_uart_handle_t *uart);
void km_io_uart_read_start(km_io_uart_handle_t *uart, uint8_t port,
                           km_io_uart_available_cb available_cb,
                           km_io_uart_read_cb read_cb);
void km_io_uart_read_stop(km_io_uart_handle_t *uart);
km_io_uart_handle_t *km_io_uart_get_by_id(uint32_t id);
void km_io_uart_cleanup();

/* idle functions */

void km_io_idle_init(km_io_idle_handle_t *idle);
void km_io_idle_start(km_io_idle_handle_t *idle, km_io_idle_cb idle_cb);
void km_io_idle_stop(km_io_idle_handle_t *idle);
km_io_idle_handle_t *km_io_idle_get_by_id(uint32_t id);
void km_io_idle_cleanup();

/* tcp functions */

void km_io_tcp_init(km_io_tcp_handle_t *tcp);
void km_io_tcp_open(km_io_tcp_handle_t *tcp);
void km_io_tcp_connect(km_io_tcp_handle_t *tcp, char *host, uint16_t port);
void km_io_tcp_read_start(km_io_tcp_handle_t *tcp,
                          km_io_tcp_available_cb available_cb,
                          km_io_tcp_read_cb read_cb);
void km_io_tcp_read_stop(km_io_tcp_handle_t *tcp);
void km_io_tcp_write(km_io_tcp_handle_t *tcp, const uint8_t *buf, size_t size);
void km_io_tcp_close(km_io_tcp_handle_t *tcp);
void km_io_tcp_cleanup();

#endif /* ___KM_IO_H */
