/* Copyright (c) 2017 Kalamu
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

#ifndef __KM_TCP_H
#define __KM_TCP_H

#include <stdint.h>

/**
 * Initialize the device driver.
 */
int km_tcp_init();

/**
 * Cleanup the device driver.
 */
int km_tcp_cleanup();

int km_tcp_socket();

int km_tcp_connect(int sock, const char* address, int port);

int km_tcp_send(int sock, const char* payload, int len);

int km_tcp_recv(int sock, char* buffer, int buffer_len);

int km_tcp_close(int sock);

typedef enum {
  KM_TCP_EVENT_CONNECT = 0,
  KM_TCP_EVENT_DISCONNECT = 1,
  KM_TCP_EVENT_READ = 2
} km_tcp_event_code_t;

typedef struct {
  km_tcp_event_code_t code;
} km_tcp_event_connect_t;

typedef struct {
  km_tcp_event_code_t code;
} km_tcp_event_disconnect_t;

typedef struct {
  km_tcp_event_code_t code;
  char* message;
  int len;
} km_tcp_event_read_t;

typedef union {
  km_tcp_event_code_t code;
  km_tcp_event_connect_t connect;
  km_tcp_event_disconnect_t disconnect;
  km_tcp_event_read_t read;
} km_tcp_event_t;

/**
 * Get WIFI event in message queue
 * 0 : if success, -1 : else
 */
int km_tcp_get_event(km_tcp_event_t* out_event);

#endif /* __KM_TCP_H */