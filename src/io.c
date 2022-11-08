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

#include "io.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "gpio.h"
#include "system.h"
#include "tty.h"
#include "uart.h"

km_io_loop_t loop;

/* forward declarations */

static void km_io_timer_run();
static void km_io_tty_run();
static void km_io_watch_run();
static void km_io_uart_run();
static void km_io_idle_run();
static void km_io_stream_run();
static void km_io_tcp_run();
static void km_io_ieee80211_run();

/* general handle functions */

uint32_t handle_id_count = 0;

void km_io_handle_init(km_io_handle_t *handle, km_io_type_t type) {
  handle->id = handle_id_count++;
  handle->type = type;
  handle->flags = 0;
  handle->close_cb = NULL;
}

void km_io_handle_close(km_io_handle_t *handle, km_io_close_cb close_cb) {
  KM_IO_SET_FLAG_ON(handle->flags, KM_IO_FLAG_CLOSING);
  handle->close_cb = close_cb;
  km_list_append(&loop.closing_handles, (km_list_node_t *)handle);
}

km_io_handle_t *km_io_handle_get_by_id(uint32_t id, km_list_t *handle_list) {
  km_io_handle_t *handle = (km_io_handle_t *)handle_list->head;
  while (handle != NULL) {
    if (handle->id == id) {
      return handle;
    }
    handle = (km_io_handle_t *)((km_list_node_t *)handle)->next;
  }
  return NULL;
}

static void km_io_update_time() { loop.time = km_gettime(); }

static void km_io_handle_closing() {
  while (loop.closing_handles.head != NULL) {
    km_io_handle_t *handle = (km_io_handle_t *)loop.closing_handles.head;
    km_list_remove(&loop.closing_handles, (km_list_node_t *)handle);
    if (handle->close_cb) {
      handle->close_cb(handle);
    }
  }
}

/* loop functions */

void km_io_init() {
  loop.stop_flag = false;
  km_io_update_time();
  km_list_init(&loop.tty_handles);
  km_list_init(&loop.timer_handles);
  km_list_init(&loop.watch_handles);
  km_list_init(&loop.uart_handles);
  km_list_init(&loop.idle_handles);
  km_list_init(&loop.stream_handles);
  km_list_init(&loop.tcp_handles);
  km_list_init(&loop.ieee80211_handles);
  km_list_init(&loop.closing_handles);
}

void km_io_cleanup() {
  km_io_timer_cleanup();
  km_io_watch_cleanup();
  km_io_uart_cleanup();
  // km_io_idle_cleanup();
  // Do not cleanup tty I/O to keep terminal communication
  km_io_stream_cleanup();
  km_io_tcp_cleanup();
  km_io_ieee80211_cleanup();
}

void km_io_run(bool infinite) {
  while (loop.stop_flag == false) {
    km_io_update_time();
    km_io_timer_run();
    km_io_tty_run();
    km_io_watch_run();
    km_io_uart_run();
    km_io_idle_run();
    km_io_stream_run();
    km_io_tcp_run();
    km_io_ieee80211_run();
    km_io_handle_closing();

    // quite if there no IO handles
    if (!infinite) {
      if (loop.timer_handles.head == NULL && loop.watch_handles.head == NULL &&
          loop.uart_handles.head == NULL && loop.idle_handles.head == NULL &&
          loop.stream_handles.head == NULL && loop.tcp_handles.head == NULL &&
          loop.ieee80211_handles.head == NULL &&
          loop.closing_handles.head == NULL) {
        loop.stop_flag = true;
      }
    }
  }
}

/* timer functions */

uint32_t timer_count = 0;

void km_io_timer_init(km_io_timer_handle_t *timer) {
  km_io_handle_init((km_io_handle_t *)timer, KM_IO_TIMER);
  timer->timer_cb = NULL;
}

void km_io_timer_start(km_io_timer_handle_t *timer, km_io_timer_cb timer_cb,
                       uint64_t interval, bool repeat) {
  KM_IO_SET_FLAG_ON(timer->base.flags, KM_IO_FLAG_ACTIVE);
  timer->timer_cb = timer_cb;
  timer->clamped_timeout = loop.time + interval;
  timer->interval = interval;
  timer->repeat = repeat;
  km_list_append(&loop.timer_handles, (km_list_node_t *)timer);
}

void km_io_timer_stop(km_io_timer_handle_t *timer) {
  KM_IO_SET_FLAG_OFF(timer->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.timer_handles, (km_list_node_t *)timer);
}

km_io_timer_handle_t *km_io_timer_get_by_id(uint32_t id) {
  return (km_io_timer_handle_t *)km_io_handle_get_by_id(id,
                                                        &loop.timer_handles);
}

void km_io_timer_cleanup() {
  km_io_timer_handle_t *handle =
      (km_io_timer_handle_t *)loop.timer_handles.head;
  while (handle != NULL) {
    km_io_timer_handle_t *next =
        (km_io_timer_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.timer_handles);
}

static void km_io_timer_run() {
  km_io_timer_handle_t *handle =
      (km_io_timer_handle_t *)loop.timer_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      if (handle->clamped_timeout < loop.time) {
        if (handle->repeat) {
          handle->clamped_timeout = handle->clamped_timeout + handle->interval;
        } else {
          KM_IO_SET_FLAG_OFF(handle->base.flags, KM_IO_FLAG_ACTIVE);
        }
        if (handle->timer_cb) {
          handle->timer_cb(handle);
        }
      }
    }
    handle = (km_io_timer_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* TTY functions */

void km_io_tty_init(km_io_tty_handle_t *tty) {
  km_io_handle_init((km_io_handle_t *)tty, KM_IO_TTY);
  tty->read_cb = NULL;
}

void km_io_tty_read_start(km_io_tty_handle_t *tty, km_io_tty_read_cb read_cb) {
  KM_IO_SET_FLAG_ON(tty->base.flags, KM_IO_FLAG_ACTIVE);
  tty->read_cb = read_cb;
  km_list_append(&loop.tty_handles, (km_list_node_t *)tty);
}

void km_io_tty_read_stop(km_io_tty_handle_t *tty) {
  KM_IO_SET_FLAG_OFF(tty->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.tty_handles, (km_list_node_t *)tty);
}

void km_io_tty_cleanup() {
  km_io_tty_handle_t *handle = (km_io_tty_handle_t *)loop.tty_handles.head;
  while (handle != NULL) {
    km_io_tty_handle_t *next =
        (km_io_tty_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.tty_handles);
}

static void km_io_tty_run() {
  km_io_tty_handle_t *handle = (km_io_tty_handle_t *)loop.tty_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      uint32_t len = km_tty_available();
      if (handle->read_cb != NULL && len > 0) {
        // for (int i = 0; i < size; i++) {
        //   handle->read_cb(km_tty_getc());
        //}
        uint8_t buf[len];
        km_tty_read(buf, len);
        handle->read_cb(buf, len);
      }
    }
    handle = (km_io_tty_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* GPIO watch functions */

void km_io_watch_init(km_io_watch_handle_t *watch) {
  km_io_handle_init((km_io_handle_t *)watch, KM_IO_WATCH);
  watch->watch_cb = NULL;
}

void km_io_watch_start(km_io_watch_handle_t *watch, km_io_watch_cb watch_cb,
                       uint8_t pin, km_io_watch_mode_t mode,
                       uint32_t debounce) {
  KM_IO_SET_FLAG_ON(watch->base.flags, KM_IO_FLAG_ACTIVE);
  watch->watch_cb = watch_cb;
  watch->pin = pin;
  watch->mode = mode;
  watch->debounce_time = 0;
  watch->debounce_delay = debounce;
  watch->last_val = (uint8_t)km_gpio_read(watch->pin);
  watch->val = (uint8_t)km_gpio_read(watch->pin);
  km_list_append(&loop.watch_handles, (km_list_node_t *)watch);
}

void km_io_watch_stop(km_io_watch_handle_t *watch) {
  KM_IO_SET_FLAG_OFF(watch->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.watch_handles, (km_list_node_t *)watch);
}

km_io_watch_handle_t *km_io_watch_get_by_id(uint32_t id) {
  return (km_io_watch_handle_t *)km_io_handle_get_by_id(id,
                                                        &loop.watch_handles);
}

void km_io_watch_cleanup() {
  km_io_watch_handle_t *handle =
      (km_io_watch_handle_t *)loop.watch_handles.head;
  while (handle != NULL) {
    km_io_watch_handle_t *next =
        (km_io_watch_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.watch_handles);
}

static void km_io_watch_run() {
  km_io_watch_handle_t *handle =
      (km_io_watch_handle_t *)loop.watch_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      uint8_t reading = (uint8_t)km_gpio_read(handle->pin);
      if (handle->last_val != reading) { /* changed by noise or pressing */
        handle->debounce_time = km_gettime();
      }
      /* debounce delay elapsed */
      uint32_t elapsed_time = km_gettime() - handle->debounce_time;
      if ((handle->watch_cb) &&
          (((handle->mode == KM_IO_WATCH_MODE_LOW_LEVEL) && (reading == 0)) ||
           ((handle->mode == KM_IO_WATCH_MODE_HIGH_LEVEL) && (reading == 1)))) {
        handle->watch_cb(handle);
      } else if (handle->debounce_time > 0 &&
                 elapsed_time >= handle->debounce_delay) {
        if (reading != handle->val) {
          handle->val = reading;
          switch (handle->mode) {
            case KM_IO_WATCH_MODE_CHANGE:
              if (handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
            case KM_IO_WATCH_MODE_RISING:
              if (handle->val == 1 && handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
            case KM_IO_WATCH_MODE_FALLING:
              if (handle->val == 0 && handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
            default:
              break;
          }
        }
        handle->debounce_time = 0;
      }
      handle->last_val = reading;
    }
    handle = (km_io_watch_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* UART functions */

void km_io_uart_init(km_io_uart_handle_t *uart) {
  km_io_handle_init((km_io_handle_t *)uart, KM_IO_UART);
}

void km_io_uart_read_start(km_io_uart_handle_t *uart, uint8_t port,
                           km_io_uart_available_cb available_cb,
                           km_io_uart_read_cb read_cb) {
  KM_IO_SET_FLAG_ON(uart->base.flags, KM_IO_FLAG_ACTIVE);
  uart->port = port;
  uart->available_cb = available_cb;
  uart->read_cb = read_cb;
  km_list_append(&loop.uart_handles, (km_list_node_t *)uart);
}

void km_io_uart_read_stop(km_io_uart_handle_t *uart) {
  KM_IO_SET_FLAG_OFF(uart->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.uart_handles, (km_list_node_t *)uart);
}

km_io_uart_handle_t *km_io_uart_get_by_id(uint32_t id) {
  return (km_io_uart_handle_t *)km_io_handle_get_by_id(id, &loop.uart_handles);
}

void km_io_uart_cleanup() {
  km_io_uart_handle_t *handle = (km_io_uart_handle_t *)loop.uart_handles.head;
  while (handle != NULL) {
    km_io_uart_handle_t *next =
        (km_io_uart_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.uart_handles);
}

static void km_io_uart_run() {
  km_io_uart_handle_t *handle = (km_io_uart_handle_t *)loop.uart_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      if (handle->available_cb != NULL && handle->read_cb != NULL) {
        int len = handle->available_cb(handle);
        if (len > 0) {
          uint8_t buf[len];
          km_uart_read(handle->port, buf, len);
          handle->read_cb(handle, buf, len);
        }
      }
    }
    handle = (km_io_uart_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* idle functions */

void km_io_idle_init(km_io_idle_handle_t *idle) {
  km_io_handle_init((km_io_handle_t *)idle, KM_IO_IDLE);
  idle->idle_cb = NULL;
}

void km_io_idle_start(km_io_idle_handle_t *idle, km_io_idle_cb idle_cb) {
  KM_IO_SET_FLAG_ON(idle->base.flags, KM_IO_FLAG_ACTIVE);
  idle->idle_cb = idle_cb;
  km_list_append(&loop.idle_handles, (km_list_node_t *)idle);
}

void km_io_idle_stop(km_io_idle_handle_t *idle) {
  KM_IO_SET_FLAG_OFF(idle->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.idle_handles, (km_list_node_t *)idle);
}

km_io_idle_handle_t *km_io_idle_get_by_id(uint32_t id) {
  return (km_io_idle_handle_t *)km_io_handle_get_by_id(id, &loop.idle_handles);
}

void km_io_idle_cleanup() {
  km_io_idle_handle_t *handle = (km_io_idle_handle_t *)loop.idle_handles.head;
  while (handle != NULL) {
    km_io_idle_handle_t *next =
        (km_io_idle_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.idle_handles);
}

static void km_io_idle_run() {
  km_io_idle_handle_t *handle = (km_io_idle_handle_t *)loop.idle_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      if (handle->idle_cb) {
        handle->idle_cb(handle);
      }
    }
    handle = (km_io_idle_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* stream function */

void km_io_stream_init(km_io_stream_handle_t *stream) {
  km_io_handle_init((km_io_handle_t *)stream, KM_IO_STREAM);
}

void km_io_stream_set_blocking(km_io_stream_handle_t *stream, bool blocking) {
  stream->blocking = blocking;
}

void km_io_stream_read_start(km_io_stream_handle_t *stream,
                             km_io_stream_available_cb available_cb,
                             km_io_stream_read_cb read_cb) {
  KM_IO_SET_FLAG_ON(stream->base.flags, KM_IO_FLAG_ACTIVE);
  stream->blocking = false;  // non-blocking
  stream->available_cb = available_cb;
  stream->read_cb = read_cb;
  km_list_append(&loop.stream_handles, (km_list_node_t *)stream);
}

void km_io_stream_read_stop(km_io_stream_handle_t *stream) {
  KM_IO_SET_FLAG_OFF(stream->base.flags, KM_IO_FLAG_ACTIVE);
  km_list_remove(&loop.stream_handles, (km_list_node_t *)stream);
}

void km_io_stream_cleanup() {
  km_io_stream_handle_t *handle =
      (km_io_stream_handle_t *)loop.stream_handles.head;
  while (handle != NULL) {
    km_io_stream_handle_t *next =
        (km_io_stream_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.stream_handles);
}

static void km_io_stream_run() {
  /*
  km_io_stream_handle_t *handle =
      (km_io_stream_handle_t *)loop.stream_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      if (!handle->blocking && handle->available_cb != NULL &&
          handle->read_cb != NULL) {
        int len = handle->available_cb(handle);
        if (len > 0) {
          uint8_t buf[len];
          // km_stream_read(handle->port, buf, len);
          handle->read_cb(handle, buf, len);
        }
      }
    }
    handle = (km_io_stream_handle_t *)((km_list_node_t *)handle)->next;
  }
  */
}

/* tcp function */

void km_io_tcp_init(km_io_tcp_handle_t *tcp) {
  km_io_handle_init((km_io_handle_t *)tcp, KM_IO_TCP);
}

int km_io_tcp_connect(km_io_tcp_handle_t *tcp, km_io_tcp_cb connect_cb) {
  return -1;
}

km_io_tcp_handle_t *km_io_tcp_get_by_id(uint32_t id) { return NULL; }

void km_io_tcp_cleanup() {
  km_io_tcp_handle_t *handle = (km_io_tcp_handle_t *)loop.tcp_handles.head;
  while (handle != NULL) {
    km_io_tcp_handle_t *next =
        (km_io_tcp_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.tcp_handles);
}

static void km_io_tcp_run() {
  km_io_tcp_handle_t *handle = (km_io_tcp_handle_t *)loop.tcp_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      // TODO
    }
    handle = (km_io_tcp_handle_t *)((km_list_node_t *)handle)->next;
  }
}

/* ieee80211 function */

void km_io_ieee80211_init(km_io_ieee80211_handle_t *ieee80211) {
  km_io_handle_init((km_io_handle_t *)ieee80211, KM_IO_IEEE80211);
}

void km_io_ieee80211_connect(km_io_ieee80211_handle_t *ieee80211,
                             km_io_ieee80211_cb connect_cb) {
  KM_IO_SET_FLAG_ON(ieee80211->base.flags, KM_IO_FLAG_ACTIVE);
  ieee80211->connect_cb = connect_cb;
  km_list_append(&loop.ieee80211_handles, (km_list_node_t *)ieee80211);
}

void km_io_ieee80211_disconnect(km_io_ieee80211_handle_t *ieee80211,
                                km_io_ieee80211_cb disconnect_cb) {}

void km_io_ieee80211_scan(km_io_ieee80211_handle_t *ieee80211,
                          km_io_ieee80211_scan_cb read_cb) {}

void km_io_ieee80211_cleanup() {
  km_io_ieee80211_handle_t *handle =
      (km_io_ieee80211_handle_t *)loop.ieee80211_handles.head;
  while (handle != NULL) {
    km_io_ieee80211_handle_t *next =
        (km_io_ieee80211_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&loop.ieee80211_handles);
}

static void km_io_ieee80211_run() {
  km_io_ieee80211_handle_t *handle =
      (km_io_ieee80211_handle_t *)loop.ieee80211_handles.head;
  while (handle != NULL) {
    if (KM_IO_HAS_FLAG(handle->base.flags, KM_IO_FLAG_ACTIVE)) {
      // int state = km_ieee80211_poll();
    }
    handle = (km_io_ieee80211_handle_t *)((km_list_node_t *)handle)->next;
  }
}
