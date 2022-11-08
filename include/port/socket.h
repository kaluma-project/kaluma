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

#ifndef __KM_SOCKET_H
#define __KM_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  KM_SOCKET_DOMAIN_TYPE_AF_INET = 0,
} km_socket_domain_type_t;

typedef enum {
  KM_SOCKET_PROTOCOL_TYPE_STREAM = 0,  // TCP
  KM_SOCKET_PROTOCOL_TYPE_DGRAM        // UDP
} km_socket_protocol_type_t;

typedef enum {
  KM_SOCKET_HOW_TYPE_SHUT_NONE = 0,
  KM_SOCKET_HOW_TYPE_SHUT_RD = 1,
  KM_SOCKET_HOW_TYPE_SHUT_WR = 2,
  KM_SOCKET_HOW_TYPE_SHUT_RDWR = 3,
} km_socket_how_type_t;

typedef struct {
  char *addr;
  uint16_t port;
} km_socket_address_t;

/**
 * Initialize
 */
void km_socket_init();

/**
 * Cleanup
 */
void km_socket_cleanup();

/**
 * Creates a socket and return a file descriptor
 * @param domain
 * @param protocol
 * @return file descriptor of socket on success, (< 0) on error (nagative
 * errno).
 */
int km_socket_create(km_socket_domain_type_t domain,
                     km_socket_protocol_type_t protocol);

/**
 * Connect the socket to the given address
 * @param fd socket fd
 * @param addr address to connect
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_connect(int fd, km_socket_address_t *addr);

/**
 * Write data to the socket
 * @param fd socket fd
 * @param buf data buffer to write
 * @param count size of buffer
 * @return number of bytes written, (< 0) on error (negative errno).
 */
int km_socket_write(int fd, uint8_t *buf, size_t count);

/**
 * Read data from the socket
 * @param fd socket fd
 * @param buf buffer to store data
 * @param count size of data to read
 * @return number of bytes read, (< 0) on error (negative errno).
 */
int km_socket_read(int fd, uint8_t *buf, size_t count);

/**
 * Close the socket
 * @param fd socket fd
 * @return 0 on success, (< 0) on error (negative errno).
 */
int km_socket_close(int fd);

/**
 * Shutdown the socket
 * @param fd socket fd
 * @param how
 * @return 0 on success, (< 0) on error (negative errno).
 */
int km_socket_shutdown(int fd, km_socket_how_type_t how);

/**
 * Bind the socket to the given address
 * @param fd socket fd
 * @param addr address to connect
 * @param port port number
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_bind(int fd, char *addr, uint16_t port);

/**
 * Listen the socket to accept incoming connections
 * @param fd socket fd
 * @return 0 on success, (< 0) on error (nagative errno).
 */
int km_socket_listen(int fd);

/**
 * Accept an incoming connection
 * @param fd socket fd
 * @param addr addr of the incoming peer socket
 * @return file descriptor of the peer socket on success, (< 0) on error
 * (nagative errno).
 */
int km_socket_accept(int fd, km_socket_address_t *addr);

#endif /* __KM_SOCKET_H */
