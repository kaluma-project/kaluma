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

#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

#include "jerryscript.h"
#ifdef KAMELEON_MODULE_IEEE80211
#include "ieee80211.h"
#endif//KAMELEON_MODULE_IEEE80211
#ifdef KAMELEON_MODULE_NET
#include "tcp.h"
#endif//KAMELEON_MODULE_NET

typedef struct io_loop_s io_loop_t;
typedef struct io_handle_s io_handle_t;
typedef struct io_handle_list_s io_handle_list_t;
typedef struct io_timer_handle_s io_timer_handle_t;
typedef struct io_tty_handle_s io_tty_handle_t;
typedef struct io_watch_handle_s io_watch_handle_t;
typedef struct io_uart_handle_s io_uart_handle_t;
#ifdef KAMELEON_MODULE_IEEE80211
typedef struct io_ieee80211_handle_s io_ieee80211_handle_t;
#endif//KAMELEON_MODULE_IEEE80211
#ifdef KAMELEON_MODULE_NET
typedef struct io_net_handle_s io_net_handle_t;
#endif//KAMELEON_MODULE_NET
typedef struct io_idle_handle_s io_idle_handle_t;

/* handle flags */

#define IO_FLAG_ACTIVE 0x01
#define IO_FLAG_CLOSING 0x02

#define IO_SET_FLAG_ON(field, flag) ((field) |= (flag))
#define IO_SET_FLAG_OFF(field, flag) ((field) &= ~(flag))
#define IO_HAS_FLAG(field, flag) ((field) & (flag))

/* general handle types */

typedef enum io_type {
  IO_TIMER,
  IO_TTY,
  IO_WATCH,
  IO_UART,
#ifdef KAMELEON_MODULE_IEEE80211  
  IO_IEEE80211,
#endif//KAMELEON_MODULE_IEEE80211
#ifdef KAMELEON_MODULE_NET  
  IO_NET,
#endif//KAMELEON_MODULE_NET
  IO_IDLE
} io_type_t;

typedef void (* io_close_cb)(io_handle_t *);

struct io_handle_s {
  list_node_t base;
  uint32_t id;
  io_type_t type;
  uint8_t flags;
  io_close_cb close_cb;
};

/* timer handle types */

typedef void (* io_timer_cb)(io_timer_handle_t *);

struct io_timer_handle_s {
  io_handle_t base;
  io_timer_cb timer_cb;
  jerry_value_t timer_js_cb;
  uint64_t clamped_timeout;
  uint64_t interval;
  bool repeat;
  uint32_t tag; // for application use
};

/* TTY handle types */

typedef void (* io_tty_read_cb)(uint8_t *, size_t);

struct io_tty_handle_s {
  io_handle_t base;
  io_tty_read_cb read_cb;
};

/* GPIO watch handle types */

typedef enum {
  IO_WATCH_MODE_FALLING,
  IO_WATCH_MODE_RISING,
  IO_WATCH_MODE_CHANGE,
} io_watch_mode_t;

/* GPIO watch handle types */

typedef enum {
  IO_PULL_NO,
  IO_PULL_UP,
  IO_PULL_DOWN,
} io_pull_t;

typedef void (* io_watch_cb)(io_watch_handle_t *);

struct io_watch_handle_s {
  io_handle_t base;
  io_watch_mode_t mode;
  uint8_t pin;
  uint64_t debounce_time;
  uint32_t debounce_delay;
  uint8_t last_val;
  uint8_t val;
  io_watch_cb watch_cb;
  jerry_value_t watch_js_cb;
};

/* UART handle type */

typedef int (* io_uart_available_cb)(io_uart_handle_t *);
typedef void (* io_uart_read_cb)(io_uart_handle_t *, uint8_t *, size_t);

struct io_uart_handle_s {
  io_handle_t base;
  uint8_t port;
  io_uart_available_cb available_cb;
  io_uart_read_cb read_cb;
  jerry_value_t read_js_cb;
  int temp;
};

/* IEEE80211 handle type */
#ifdef KAMELEON_MODULE_IEEE80211
typedef void (* io_ieee80211_connect_cb)(io_ieee80211_handle_t *);
typedef void (* io_ieee80211_assoc_cb)(io_ieee80211_handle_t *);
typedef void (* io_ieee80211_disconnect_cb)(io_ieee80211_handle_t *);
typedef void (* io_ieee80211_scan_cb)(io_ieee80211_handle_t *, int, ieee80211_scan_info_t*);

struct io_ieee80211_handle_s {
  io_handle_t base;
  uint8_t port;
  io_ieee80211_scan_cb scan_cb;
  io_ieee80211_assoc_cb assoc_cb;
  io_ieee80211_connect_cb connect_cb;
  io_ieee80211_disconnect_cb disconnect_cb;
  jerry_value_t this_val;
  jerry_value_t scan_js_cb;
};
#endif//KAMELEON_MODULE_IEEE80211

#ifdef KAMELEON_MODULE_NET
typedef void (* io_net_connect_cb)(io_net_handle_t *);
typedef void (* io_net_disconnect_cb)(io_net_handle_t *);
typedef void (* io_net_read_cb)(io_net_handle_t *, const char*, int);

struct io_net_handle_s {
  io_handle_t base;
  io_net_connect_cb connect_cb;
  io_net_disconnect_cb disconnect_cb;
  io_net_read_cb read_cb;
  jerry_value_t this_val;
  int fd;
};
#endif//KAMELEON_MODULE_NET

/* idle handle types */

typedef void (* io_idle_cb)(io_idle_handle_t *);

struct io_idle_handle_s {
  io_handle_t base;
  io_idle_cb idle_cb;
};

/* loop type */

struct io_loop_s {
  bool stop_flag;
  uint64_t time;
  list_t timer_handles;
  list_t tty_handles;
  list_t watch_handles;
  list_t uart_handles;
#ifdef KAMELEON_MODULE_IEEE80211  
  list_t ieee80211_handles;
#endif//KAMELEON_MODULE_IEEE80211
#ifdef KAMELEON_MODULE_NET  
  list_t net_handles;
#endif//KAMELEON_MODULE_NET
  list_t idle_handles;
  list_t closing_handles;
};

/* loop functions */

void io_init();
void io_run();

/* general handle functions */

void io_handle_init(io_handle_t *handle, io_type_t type);
void io_handle_close(io_handle_t *handle, io_close_cb close_cb);
io_handle_t *io_handle_get_by_id(uint32_t id, list_t *handle_list);

/* timer functions */

void io_timer_init(io_timer_handle_t *timer);
void io_timer_start(io_timer_handle_t *timer, io_timer_cb timer_cb, uint64_t interval, bool repeat);
void io_timer_stop(io_timer_handle_t *timer);
io_timer_handle_t *io_timer_get_by_id(uint32_t id);
void io_timer_cleanup();

/* TTY functions */

void io_tty_init(io_tty_handle_t *tty);
void io_tty_read_start(io_tty_handle_t *tty, io_tty_read_cb read_cb);
void io_tty_read_stop(io_tty_handle_t *tty);
void io_tty_cleanup();

/* GPIO watch functions */

void io_watch_init(io_watch_handle_t *watch);
void io_watch_start(io_watch_handle_t *watch, io_watch_cb watch_cb, uint8_t pin, io_watch_mode_t mode, uint32_t debounce);
void io_watch_stop(io_watch_handle_t *watch);
io_watch_handle_t *io_watch_get_by_id(uint32_t id);
void io_watch_cleanup();

/* UART function */

void io_uart_init(io_uart_handle_t *uart);
void io_uart_read_start(io_uart_handle_t *uart, uint8_t port, io_uart_available_cb available_cb, io_uart_read_cb read_cb);
void io_uart_read_stop(io_uart_handle_t *uart);
io_uart_handle_t *io_uart_get_by_id(uint32_t id);
void io_uart_cleanup();

/* IEEE80211 function */
#ifdef KAMELEON_MODULE_IEEE80211
void io_ieee80211_init(io_ieee80211_handle_t *ieee80211);
void io_ieee80211_start(io_ieee80211_handle_t *ieee80211, io_ieee80211_scan_cb scan_cb, io_ieee80211_assoc_cb assoc_cb, io_ieee80211_connect_cb connect_cb, io_ieee80211_disconnect_cb disconnect_cb);
void io_ieee80211_stop(io_ieee80211_handle_t *ieee80211);
void io_ieee80211_scan(io_ieee80211_handle_t *ieee80211, io_ieee80211_scan_cb scan_cb);
void io_ieee80211_connect(io_ieee80211_handle_t *ieee80211, io_ieee80211_connect_cb connect_cb);
void io_ieee80211_disconnect(io_ieee80211_handle_t *ieee80211, io_ieee80211_disconnect_cb disconnect_cb);
io_ieee80211_handle_t *io_ieee80211_get_by_id(uint32_t id);
void io_ieee80211_cleanup();
#endif//KAMELEON_MODULE_IEEE80211
#ifdef KAMELEON_MODULE_NET
void io_net_init(io_net_handle_t *net);
void io_net_start(io_net_handle_t *net, io_net_connect_cb connect_cb, io_net_disconnect_cb disconnect_cb, io_net_read_cb read_cb);
void io_net_stop(io_net_handle_t *net);
io_net_handle_t *io_net_get_by_fd(int fd);
void io_net_cleanup();
#endif//KAMELEON_MODULE_NET
/* idle function */

void io_idle_init(io_idle_handle_t *idle);
void io_idle_start(io_idle_handle_t *idle, io_idle_cb idle_cb);
void io_idle_stop(io_idle_handle_t *idle);
io_idle_handle_t *io_idle_get_by_id(uint32_t id);
void io_idle_cleanup();

#endif /* __IO_H */
