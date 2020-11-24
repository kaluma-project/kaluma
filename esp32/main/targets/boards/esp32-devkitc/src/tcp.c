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
#include <lwip/sockets.h>
#include <esp_log.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <sys/errno.h>

#include "tcp.h"
#define TAG "KAMELEON_TCP"
#define MAX_SOCKET_NUM (1)
#define INVALID_FD_VALUE (-1)

static QueueHandle_t s_message_queue;
static int socket_fds[MAX_SOCKET_NUM];

static void create_connect_event(void);
static void create_disconnect_event(void);
static void create_read_event(const char* buf, int len);

int kameleon_tcp_init()
{
  for(int i = 0 ; i < MAX_SOCKET_NUM ; i++ )
  {
    socket_fds[i] = INVALID_FD_VALUE;
  }

  s_message_queue = xQueueCreate(2, sizeof(tcp_event_t));
  return 0;
}

/**
 * Cleanup the device driver.
 */
int kameleon_tcp_cleanup()
{
  vQueueDelete(s_message_queue);
  return 0;
}

static int add_socket_list(int fd)
{
  for(int i = 0 ; i < MAX_SOCKET_NUM ; i++ )
  {
    if (socket_fds[i] == -1) {
      socket_fds[i] = fd;
      ESP_LOGI(TAG, "socket_fds[%d] <- %d", i, fd);
      return 0;
    }
  }
  return -1;  
}

static void remove_socket_list(int fd)
{
  ESP_LOGI(TAG, "remove_socket_list(%d)", fd);
  for(int i = 0 ; i < MAX_SOCKET_NUM ; i++ )
  {
    if (socket_fds[i] == fd) {
      socket_fds[i] = INVALID_FD_VALUE;
      return;
    }
  }
}


int kameleon_tcp_socket()
{
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  ESP_LOGI(TAG, "kameleon_tcp_socket : %d", fd);
  if ( fd == INVALID_FD_VALUE ) return fd;

  if ( add_socket_list(fd) == -1 )
  {
    ESP_LOGE(TAG, "MAX_SOCKET_NUM");
    close(fd);
    return -1;
  }
  return fd;
}


int kameleon_tcp_connect(int sock, const char* address, int port)
{
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(address);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  int ret = connect(sock, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in));
  ESP_LOGI(TAG, "kameleon_tcp_connect : %d", ret);
  create_connect_event();
  return ret;
}

static void create_connect_event(void)
{
    ESP_LOGD(TAG, "create_connect_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    tcp_event_t message;
    message.code = TCP_EVENT_CONNECT;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

int kameleon_tcp_send(int sock, const char* payload, int len)
{
  int ret= send(sock, payload, len, 0);
  ESP_LOGI(TAG, "kameleon_tcp_send : %d", ret);
  return ret;
}


int kameleon_tcp_recv(int sock, char* buffer, int buffer_len)
{
  int ret= recv(sock, buffer, buffer_len, 0);
  ESP_LOGI(TAG, "kameleon_tcp_recv : %d", ret);
  return ret;
}

static void create_read_event(const char* buf, int len)
{
    ESP_LOGD(TAG, "create_read_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    tcp_event_t message;
    message.code = TCP_EVENT_READ;
    message.read.message = buf;
    message.read.len = len;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

int kameleon_tcp_close(int sock)
{
  int ret= close(sock);
  if(ret == 0)
  {
    remove_socket_list(sock);
  }
  ESP_LOGI(TAG, "kameleon_tcp_close : %d", ret);
  create_disconnect_event();
  return ret;
}

static void create_disconnect_event(void)
{
    ESP_LOGD(TAG, "create_close_event");
    static const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    tcp_event_t message;
    message.code = TCP_EVENT_DISCONNECT;
    xQueueSendToFront(s_message_queue, &message, xTicksToWait);
}

int kameleon_tcp_get_event(tcp_event_t* out_event)
{
  fd_set rdfs;
  int max_socket_fd = INVALID_FD_VALUE;
  FD_ZERO(&rdfs);
  for(int i = 0 ; i < MAX_SOCKET_NUM ; i++ )
  {
    if ( socket_fds[i] == INVALID_FD_VALUE ) continue;
    FD_SET(socket_fds[i], &rdfs);
    if ( max_socket_fd < socket_fds[i] )
    {
      max_socket_fd = socket_fds[i];
    }
  }

  if ( max_socket_fd != INVALID_FD_VALUE )
  {
    struct timeval tv = {
      .tv_sec = 0,
      .tv_usec = 1E3
    };
  
    int s = select(max_socket_fd+1, &rdfs, NULL, NULL, &tv );
    if ( s < 0 )
    {
      ESP_LOGE(TAG, "select failed %d", errno);
    }
    else if ( s > 0 )
    {
      for(int i = 0 ; i < MAX_SOCKET_NUM ; i++)
      {
        if(FD_ISSET(socket_fds[i], &rdfs))
        {
          int len = 100;
          //ioctl(socket_fds[i], FIONREAD, &len);
          //ESP_LOGI(TAG, "len: %d", len);
          if ( len > 0 )
          {
            char* buf = malloc(len);
            int read_bytes = read(socket_fds[i], buf, len-1);
            if(read_bytes > 0) {
              buf[read_bytes] = 0;
              create_read_event(buf, read_bytes);
            }
            else if ( read_bytes == 0 )
            {
              kameleon_tcp_close(socket_fds[i]);
            }
            else {
              ESP_LOGE(TAG, "read error %d", read_bytes);
            }
          }
        }
      }
    }
  }

  BaseType_t ret = xQueueReceive(s_message_queue, out_event, 0);
  if ( ret != pdTRUE ) return -1;
  return 0;
}
