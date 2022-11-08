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

#include "socket.h"

#include <stdlib.h>

#include "board.h"
#include "err.h"

/**
 * Initialize
 */
void km_socket_init() {}

/**
 * Cleanup
 */
void km_socket_cleanup() {}

/**
 * Creates a socket and return a file descriptor
 * @param domain
 * @param protocol
 * @return file descriptor of socket on success, (< 0) on error (nagative
 * errno).
 */
int km_socket_create(km_socket_domain_type_t domain,
                     km_socket_protocol_type_t protocol) {
  return -1;
}

/**
 * Connect the socket to the given address
 * @param fd socket fd
 * @param addr address to connect
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_connect(int fd, km_socket_address_t *addr) { return -1; }

/**
 * Write data to the socket
 * @param fd socket fd
 * @param buf data buffer to write
 * @param count size of buffer
 * @return number of bytes written, (< 0) on error (negative errno).
 */
int km_socket_write(int fd, uint8_t *buf, size_t count) { return -1; }

/**
 * Read data from the socket
 * @param fd socket fd
 * @param buf buffer to store data
 * @param count size of data to read
 * @return number of bytes read, (< 0) on error (negative errno).
 */
int km_socket_read(int fd, uint8_t *buf, size_t count) { return -1; }

/**
 * Close the socket
 * @param fd socket fd
 * @return 0 on success, (< 0) on error (negative errno).
 */
int km_socket_close(int fd) { return -1; }

/**
 * Shutdown the socket
 * @param fd socket fd
 * @param how
 * @return 0 on success, (< 0) on error (negative errno).
 */
int km_socket_shutdown(int fd, km_socket_how_type_t how) { return -1; }

/**
 * Bind the socket to the given address
 * @param fd socket fd
 * @param addr address to connect
 * @param port port number
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_bind(int fd, char *addr, uint16_t port) { return -1; }

/**
 * Listen the socket to accept incoming connections
 * @param fd socket fd
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_listen(int fd) { return -1; }

/**
 * Accept an incoming connection
 * @param fd socket fd
 * @param addr addr of the incoming peer socket
 * @return file descriptor of the peer socket on success, (< 0) on error
 * (nagative errno).
 */
int km_socket_accept(int fd, km_socket_address_t *addr) { return -1; }
