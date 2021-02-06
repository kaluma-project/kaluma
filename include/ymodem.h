#ifndef __KM_YMODEM_H_
#define __KM_YMODEM_H_

typedef enum {
  KM_YMODEM_OK = 0,
  KM_YMODEM_ERROR,
  KM_YMODEM_ABORT,
  KM_YMODEM_TIMEOUT,
  KM_YMODEM_DATA,
  KM_YMODEM_LIMIT
} km_ymodem_status_t;

typedef int (* km_ymodem_header_cb)(uint8_t *file_name, size_t file_size);
typedef int (* km_ymodem_packet_cb)(uint8_t *data, size_t len);
typedef void (* km_ymodem_footer_cb)();
km_ymodem_status_t km_ymodem_receive(km_ymodem_header_cb header_cb, km_ymodem_packet_cb packet_cb, km_ymodem_footer_cb footer_cb);

#endif  /* __KM_YMODEM_H_ */
