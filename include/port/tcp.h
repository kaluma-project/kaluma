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

#ifndef __KM_TCP_H
#define __KM_TCP_H

#include <stddef.h>
#include <stdint.h>

void km_tcp_init();
void km_tcp_cleanup();
int km_tcp_open();
int km_tcp_connect(int fd, char *host, uint16_t port);
int km_tcp_write(int fd, const uint8_t *buf, size_t len);
int km_tcp_close(int fd);
int km_tcp_shutdown(int fd, int how);
int km_tcp_bind(int fd, char *host, uint16_t port);
int km_tcp_listen(int fd, int backlog);
int km_tcp_accept(int fd, char *host, uint16_t *port);
int km_tcp_read(int fd, uint8_t *buf, size_t len); // read from read buffrer and return number of byte read;

// returns TCP_POLL_CONNECTED | TCP_POLL_READ | TCP_POLL_ACCEPTED | ...
int km_tcp_io_poll(int fd);

#endif /* __KM_TCP_H */
