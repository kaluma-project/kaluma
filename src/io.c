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
#include "gpio.h"
#include "uart.h"

io_loop_t loop;

/* forward declarations */

static void io_timer_run();
static void io_tty_run();
static void io_watch_run();
static void io_uart_run();
static void io_idle_run();

/* general handle functions */

uint32_t handle_id_count = 0;

void io_handle_init(io_handle_t *handle, io_type_t type) {
  handle->id = handle_id_count++;
  handle->type = type;
  handle->flags = 0;
  handle->close_cb = NULL;
}

void io_handle_close(io_handle_t *handle, io_close_cb close_cb) {
  IO_SET_FLAG_ON(handle->flags, IO_FLAG_CLOSING);
  handle->close_cb = close_cb;
  list_append(&loop.closing_handles, (list_node_t *) handle);
}

io_handle_t *io_handle_get_by_id(uint32_t id, list_t *handle_list) {
  io_handle_t *handle = (io_handle_t *) handle_list->head;
  while (handle != NULL) {
    if (handle->id == id) {
      return handle;
    }
    handle = (io_handle_t *) ((list_node_t *) handle)->next;
  }
  return NULL;
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
  list_init(&loop.timer_handles);
  list_init(&loop.watch_handles);
  list_init(&loop.uart_handles);
  list_init(&loop.closing_handles);
}

void io_run() {
  while (loop.stop_flag == false) {
    io_update_time();
    io_timer_run();
    io_tty_run();
    io_watch_run();
    io_uart_run();
    io_idle_run();
    io_handle_closing();
  }
}

/* timer functions */

uint32_t timer_count = 0;

void io_timer_init(io_timer_handle_t *timer) {
  io_handle_init((io_handle_t *) timer, IO_TIMER);
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

io_timer_handle_t *io_timer_get_by_id(uint32_t id) {
  return (io_timer_handle_t *) io_handle_get_by_id(id, &loop.timer_handles);
}

void io_timer_cleanup() {
  io_timer_handle_t *handle = (io_timer_handle_t *) loop.timer_handles.head;
  while (handle != NULL) {
    io_timer_handle_t *next = (io_timer_handle_t *) ((list_node_t *) handle)->next;
    free(handle);
    handle = next;
  }
  list_init(&loop.timer_handles);
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

void io_tty_cleanup() {
  io_tty_handle_t *handle = (io_tty_handle_t *) loop.tty_handles.head;
  while (handle != NULL) {
    io_tty_handle_t *next = (io_tty_handle_t *) ((list_node_t *) handle)->next;
    free(handle);
    handle = next;
  }
  list_init(&loop.tty_handles);
}

static void io_tty_run() {
  io_tty_handle_t *handle = (io_tty_handle_t *) loop.tty_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      uint32_t len = tty_available();
      if (handle->read_cb != NULL && len > 0) {
        // for (int i = 0; i < size; i++) {
        //   handle->read_cb(tty_getc());
        //}
        uint8_t buf[len];
        tty_read(buf, len);
        handle->read_cb(buf, len);
      }
    }
    handle = (io_tty_handle_t *) ((list_node_t *) handle)->next;
  }
}

/* GPIO watch functions */

void io_watch_init(io_watch_handle_t *watch) {
  io_handle_init((io_handle_t *) watch, IO_WATCH);
  watch->watch_cb = NULL;
}

uint8_t io_watch_start(io_watch_handle_t *watch, io_watch_cb watch_cb, uint8_t pin, io_watch_mode_t mode, uint32_t debounce) {
  IO_SET_FLAG_ON(watch->base.flags, IO_FLAG_ACTIVE);
  if (gpio_set_io_mode(pin, GPIO_IO_MODE_INPUT) == GPIOPORT_ERROR)
    return GPIOPORT_ERROR;
  watch->watch_cb = watch_cb;
  watch->pin = pin;
  watch->mode = mode;
  watch->debounce_time = 0;
  watch->debounce_delay = debounce;
  watch->last_val = (uint8_t)gpio_read(watch->pin);
  watch->val = (uint8_t)gpio_read(watch->pin);
  list_append(&loop.watch_handles, (list_node_t *) watch);
  return 0;
}

void io_watch_stop(io_watch_handle_t *watch) {
  IO_SET_FLAG_OFF(watch->base.flags, IO_FLAG_ACTIVE);
  list_remove(&loop.watch_handles, (list_node_t *) watch);
}

io_watch_handle_t *io_watch_get_by_id(uint32_t id) {
  return (io_watch_handle_t *) io_handle_get_by_id(id, &loop.watch_handles);
}

void io_watch_cleanup() {
  io_watch_handle_t *handle = (io_watch_handle_t *) loop.watch_handles.head;
  while (handle != NULL) {
    io_watch_handle_t *next = (io_watch_handle_t *) ((list_node_t *) handle)->next;
    free(handle);
    handle = next;
  }
  list_init(&loop.watch_handles);
}

static void io_watch_run() {
  io_watch_handle_t *handle = (io_watch_handle_t *) loop.watch_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      uint8_t reading = (uint8_t)gpio_read(handle->pin);
      if (handle->last_val != reading) { /* changed by noise or pressing */
        handle->debounce_time = gettime();
      }
      /* debounce delay elapsed */
      uint32_t elapsed_time = gettime() - handle->debounce_time;
      if (handle->debounce_time > 0 && elapsed_time >= handle->debounce_delay) {
        if (reading != handle->val) {
          handle->val = reading;
          switch (handle->mode) {
            case IO_WATCH_MODE_CHANGE:
              if (handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
            case IO_WATCH_MODE_RISING:
              if (handle->val == 1 && handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
            case IO_WATCH_MODE_FALLING:
              if (handle->val == 0 && handle->watch_cb) {
                handle->watch_cb(handle);
              }
              break;
          }
        }
        handle->debounce_time = 0;
      }
      handle->last_val = reading;
    }
    handle = (io_watch_handle_t *) ((list_node_t *) handle)->next;
  }
}

/* UART functions */

void io_uart_init(io_uart_handle_t *uart) {
  io_handle_init((io_handle_t *) uart, IO_UART);
}

void io_uart_read_start(io_uart_handle_t *uart, uint8_t port, io_uart_available_cb available_cb, io_uart_read_cb read_cb) {
  IO_SET_FLAG_ON(uart->base.flags, IO_FLAG_ACTIVE);
  uart->port = port;
  uart->available_cb = available_cb;
  uart->read_cb = read_cb;
  list_append(&loop.uart_handles, (list_node_t *) uart);
}

void io_uart_read_stop(io_uart_handle_t *uart) {
  IO_SET_FLAG_OFF(uart->base.flags, IO_FLAG_ACTIVE);
  list_remove(&loop.uart_handles, (list_node_t *) uart);
}

io_uart_handle_t *io_uart_get_by_id(uint32_t id) {
  return (io_uart_handle_t *) io_handle_get_by_id(id, &loop.uart_handles);
}

void io_uart_cleanup() {
  io_uart_handle_t *handle = (io_uart_handle_t *) loop.uart_handles.head;
  while (handle != NULL) {
    io_uart_handle_t *next = (io_uart_handle_t *) ((list_node_t *) handle)->next;
    free(handle);
    handle = next;
  }
  list_init(&loop.uart_handles);
}

static void io_uart_run() {
  io_uart_handle_t *handle = (io_uart_handle_t *) loop.uart_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      if (handle->available_cb != NULL && handle->read_cb != NULL) {
        int len = handle->available_cb(handle);
        if (len > 0) {
          uint8_t buf[len];
          uart_read(handle->port, buf, len);
          handle->read_cb(handle, buf, len);
        }
      }
    }
    handle = (io_uart_handle_t *) ((list_node_t *) handle)->next;
  }
}

/* idle functions */

void io_idle_init(io_idle_handle_t *idle) {
  io_handle_init((io_handle_t *) idle, IO_IDLE);
  idle->idle_cb = NULL;
}

void io_idle_start(io_idle_handle_t *idle, io_idle_cb idle_cb) {
  IO_SET_FLAG_ON(idle->base.flags, IO_FLAG_ACTIVE);
  idle->idle_cb = idle_cb;
  list_append(&loop.idle_handles, (list_node_t *) idle);
}

void io_idle_stop(io_idle_handle_t *idle) {
  IO_SET_FLAG_OFF(idle->base.flags, IO_FLAG_ACTIVE);
  list_remove(&loop.idle_handles, (list_node_t *) idle);
}

io_idle_handle_t *io_idle_get_by_id(uint32_t id) {
  return (io_idle_handle_t *) io_handle_get_by_id(id, &loop.idle_handles);
}

void io_idle_cleanup() {
  io_idle_handle_t *handle = (io_idle_handle_t *) loop.idle_handles.head;
  while (handle != NULL) {
    io_idle_handle_t *next = (io_idle_handle_t *) ((list_node_t *) handle)->next;
    free(handle);
    handle = next;
  }
  list_init(&loop.idle_handles);
}

static void io_idle_run() {
  io_idle_handle_t *handle = (io_idle_handle_t *) loop.idle_handles.head;
  while (handle != NULL) {
    if (IO_HAS_FLAG(handle->base.flags, IO_FLAG_ACTIVE)) {
      if (handle->idle_cb) {
        handle->idle_cb(handle);
      }
    }
    handle = (io_idle_handle_t *) ((list_node_t *) handle)->next;
  }
}
