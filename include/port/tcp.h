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

#pragma once

#include <stdint.h>

/**
 * Initialize the device driver.
 */
int kameleon_tcp_init();

/**
 * Cleanup the device driver.
 */
int kameleon_tcp_cleanup();

int kameleon_tcp_socket();

int kameleon_tcp_connect(int sock, const char* address, int port);

int kameleon_tcp_send(int sock, const char* payload, int len);

int kameleon_tcp_recv(int sock, char* buffer, int buffer_len);

int kameleon_tcp_close(int sock);

typedef enum {
  TCP_EVENT_CONNECT = 0,
  TCP_EVENT_DISCONNECT = 1,
  TCP_EVENT_READ = 2
} tcp_event_code_t;

typedef struct {
  tcp_event_code_t code;
} tcp_event_connect_t;

typedef struct {
  tcp_event_code_t code;
} tcp_event_disconnect_t;

typedef struct {
  tcp_event_code_t code;
  char* message;
  int len;
} tcp_event_read_t;

typedef union {
  tcp_event_code_t code;
  tcp_event_connect_t connect;
  tcp_event_disconnect_t disconnect;
  tcp_event_read_t read;
} tcp_event_t;

/**
 * Get WIFI event in message queue
 * 0 : if success, -1 : else
 */
int kameleon_tcp_get_event(tcp_event_t* out_event);

