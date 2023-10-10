# Proposal for V2

- Improve Async I/O
- Faster code upload

## Improve Async I/O

- 문제점
  - 현재 `net`, `http` 모듈이 `io.h`를 거치지 않고 단독으로 구현됨.
  - `global.__netdev`, `global.__ieee80211dev`의 잠재적 문제점이 많다.

- 해결책 후보
  - `global.__netdev`, `global.__ieee80211dev`를 제거
  - `tcp.h`, `udp.h`, `ieee80211.h` 인터페이스를 `include/port`에 정의
  - `tcp.h`는 polling 방식으로 사용할 수 있도록 함수들을 정의한다.
  - `tcp.h` 등을 각 target(rp2, ...)에서 구현할때, 일반적인 socket 함수들은 blocking 방식이므로, lwip의 callback-style API를 사용한다.

### ex) TCP

```c
// include/port/tcp.h

#define TCP_POLL_CONNECTED 1
#define TCP_POLL_DATA_IN   2
#define TCP_POLL_DATA_OUT  4
#define TCP_POLL_ACCEPTED  8
#define TCP_POLL_CLOSED    16

void km_tcp_init();
void km_tcp_cleanup();
int km_tcp_open(); // return fd
int km_tcp_connect(int fd, char *host, uint16_t port);
int km_tcp_write(int fd, const uint8_t *buf, size_t len);
int km_tcp_close(int fd);
int km_tcp_shutdown(int fd, int how);
int km_tcp_bind(int fd, char *host, uint16_t port);
int km_tcp_listen(int fd, int backlog);
int km_tcp_accept(int fd, char *host, uint16_t *port);
int km_tcp_read(int fd, uint8_t *buf, size_t len) // read from read buffrer and return number of byte read;

// returns flags of events
int km_tcp_io_poll();
```

```c
// io.h

typedef void (*km_io_tcp_cb)(km_io_tcp_handle_t *tcp);
typedef void (*km_io_tcp_connect_cb)(km_io_tcp_handle_t *tcp, int fd);
typedef void (*km_io_tcp_accept_cb)(km_io_tcp_handle_t *tcp);
typedef int (*km_io_tcp_available_cb)(km_io_tcp_handle_t *tcp);
typedef void (*km_io_tcp_read_cb)(km_io_tcp_handle_t *tcp, uint8_t buf*, size_t len);

struct km_io_tcp_handle_s {
  KM_IO_HANDLE_FIELDS
  int fd;                                  
  km_io_tcp_connect_cb tcp_connect_cb;     
  km_io_tcp_accept_cb tcp_accept_cb;       
  km_io_tcp_available_cb tcp_available_cb;
  km_io_tcp_read_cb tcp_read_cb;           
  km_io_tcp_cb tcp_close_cb;
};

void km_io_tcp_init(km_io_tcp_handle_t *tcp);
void km_io_tcp_open(km_io_tcp_handle_t *tcp);
void km_io_tcp_connect(km_io_tcp_handle_t *tcp, char *host, uint16_t port, connect_cb);
void km_io_tcp_read_start(km_io_tcp_handle_t *tcp, km_io_tcp_available_cb available_cb, km_io_tcp_read_cb read_cb);
void km_io_tcp_read_stop(km_io_tcp_handle_t *tcp);
void km_io_tcp_write(km_io_tcp_handle_t *tcp, const uint8_t *buf, size_t size);
void km_io_tcp_close(km_io_tcp_handle_t *tcp);
void km_io_tcp_cleanup();
```

```c
// targets/rp2/src/tcp.c

typedef struct {
  int fd;
  int event_flags = 0;
  uint8_t *read_buf;
  int read_buf_len = 0;
  uint8_t *write_buf;
  int write_buf_len = 0;
  tcp_object_t *next;
} tcp_object_t;

tcp_object_t *tcp_object_list;

static int __tcp_connect_cb(int *arg) {
  int *fd = (int *)arg;
  tcp_object_t *obj = find_tcp_object_by_fd(fd);
  obj->event_flags &= TCP_POLL_CONNECTED;
}

static int __tcp_recv(int *arg, unit8_t *buf, size_t len) {
  int *fd = (int *)arg;
  tcp_object_t *obj = find_tcp_object_by_fd(fd);
  obj->read_buf_len += len;
  // write buf to obj-read_buf
  obj->event_flags &= TCP_POLL_DATA_IN;
}

int km_tcp_connect(int fd, char *host, uint16_t port) {
  lwip_tcp_connect(fd, host, port, __connect_cb);
}

int km_tcp_read(int fd, unit8_t *buf, size_t len) {
  int *fd = (int *)arg;
  tcp_object_t *obj = find_tcp_object_by_fd(fd);
  // copy obj->read_buf to buf
  obj->read_buf_len = 0;
}

// 한번에 여러 event가 발생할 가능성 있음 --> event_flags 방식으로 해야 하나?
int km_tcp_poll(int fd) {
  tcp_object_t *obj = find_tcp_object_by_fd(fd);
  int events = obj->event_flags;
  tcp_obj->event_flags = 0;
  return events;
}
```

```c
// io.c

static void km_io_tcp_run()
{
  km_io_tcp_handle_t *handle = (km_io_tcp_handle_t *)loop.tcp_handles.head;
  while (handle != NULL)
  {
    if (KM_IO_HAS_FLAG(handle->flags, KM_IO_FLAG_ACTIVE))
    {
      int fd = handle->fd;
      int event = km_tcp_poll(fd);
      switch (event) {
        case TCP_POLL_CONNECTED:
          if (handle->connect_cb) handle->connect_cb(handle);
          break;
        case TCP_POLL_DATA_IN:
          uint8_t *buf;
          size_t len = km_tcp_read(fd, buf);
          if (handle->read_cb) handle->read_cb(handle, buf, len);
          break;
        case TCP_POLL_ACCEPTED:
          // ...
          break;
      }
    }
    handle = (km_io_tcp_handle_t *)((km_list_node_t *)handle)->next;
  }
}
```
