#include "ymodem.h"

#include <stdlib.h>

#include "tty.h"

#define PACKET_HEADER_SIZE ((uint32_t)3)
#define PACKET_DATA_INDEX ((uint32_t)4)
#define PACKET_START_INDEX ((uint32_t)1)
#define PACKET_NUMBER_INDEX ((uint32_t)2)
#define PACKET_CNUMBER_INDEX ((uint32_t)3)
#define PACKET_TRAILER_SIZE ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE ((uint32_t)128)
#define PACKET_1K_SIZE ((uint32_t)1024)

/* /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons                 */

#define FILE_NAME_LENGTH ((uint32_t)64)
#define FILE_SIZE_LENGTH ((uint32_t)16)

#define SOH ((uint8_t)0x01)   /* start of 128-byte data packet */
#define STX ((uint8_t)0x02)   /* start of 1024-byte data packet */
#define EOT ((uint8_t)0x04)   /* end of transmission */
#define ACK ((uint8_t)0x06)   /* acknowledge */
#define NAK ((uint8_t)0x15)   /* negative acknowledge */
#define CA ((uint32_t)0x18)   /* two of these in succession aborts transfer */
#define CRC16 ((uint8_t)0x43) /* 'C' == 0x43, request 16-bit CRC */
#define NEGATIVE_BYTE ((uint8_t)0xFF)

#define ABORT1 ((uint8_t)0x41) /* 'A' == 0x41, abort by user */
#define ABORT2 ((uint8_t)0x61) /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT ((uint32_t)0x100000)
#define DOWNLOAD_TIMEOUT ((uint32_t)1000) /* One second retry delay */
#define MAX_ERRORS ((uint32_t)5)

uint8_t packet_data[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint32_t session_done, session_begin;

static km_ymodem_status_t receive_packet(uint8_t *data, uint32_t *length,
                                         uint32_t timeout);
static uint16_t update_crc16(uint16_t crc_in, uint8_t byte);
static uint16_t calc_crc16(const uint8_t *p_data, uint32_t size);

/**
 * @brief  Update CRC16 for input byte
 * @param  crc_in input value
 * @param  input byte
 * @return None
 */
static uint16_t update_crc16(uint16_t crc_in, uint8_t byte) {
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;
  do {
    crc <<= 1;
    in <<= 1;
    if (in & 0x100) ++crc;
    if (crc & 0x10000) crc ^= 0x1021;
  } while (!(in & 0x10000));
  return crc & 0xffffu;
}

/**
 * @brief  Calculate CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @return None
 */
static uint16_t calc_crc16(const uint8_t *data, uint32_t size) {
  uint32_t crc = 0;
  const uint8_t *dataEnd = data + size;
  while (data < dataEnd) {
    crc = update_crc16(crc, *data++);
  }
  crc = update_crc16(crc, 0);
  crc = update_crc16(crc, 0);
  return crc & 0xffffu;
}

/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @return KM_YMODEM_OK: normally return
 *         KM_YMODEM_ABORT: aborted by user
 */
static km_ymodem_status_t receive_packet(uint8_t *data, uint32_t *length,
                                         uint32_t timeout) {
  uint32_t crc;
  uint32_t packet_size = 0;
  km_ymodem_status_t status;
  uint8_t ch;
  uint32_t ret;

  *length = 0;
  ret = km_tty_read_sync(&ch, 1, timeout);
  if (ret > 0) {
    status = KM_YMODEM_OK;
  } else {
    status = KM_YMODEM_TIMEOUT;
  }

  if (status == KM_YMODEM_OK) {
    switch (ch) {
      case SOH:
        packet_size = PACKET_SIZE;
        break;
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
      case EOT:
        break;
      case CA:
        ret = km_tty_read_sync(&ch, 1, timeout);
        if ((ret > 0) && (ch == CA)) {
          packet_size = 2;
        } else {
          status = KM_YMODEM_ERROR;
        }
        break;
      case ABORT1:
      case ABORT2:
        status = KM_YMODEM_ABORT;
        break;
      default:
        status = KM_YMODEM_ERROR;
        break;
    }
    *data = ch;

    if (packet_size >= PACKET_SIZE) {
      ret = km_tty_read_sync(&data[PACKET_NUMBER_INDEX],
                             packet_size + PACKET_OVERHEAD_SIZE, timeout);
      if (ret > 0) {
        status = KM_YMODEM_OK;
      } else {
        status = KM_YMODEM_TIMEOUT;
      }

      /* Simple packet sanity check */
      if (status == KM_YMODEM_OK) {
        if (data[PACKET_NUMBER_INDEX] !=
            ((data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
          packet_size = 0;
          status = KM_YMODEM_ERROR;
        } else {
          /* Check packet CRC */
          crc = data[packet_size + PACKET_DATA_INDEX] << 8;
          crc += data[packet_size + PACKET_DATA_INDEX + 1];
          if (calc_crc16(&data[PACKET_DATA_INDEX], packet_size) != crc) {
            packet_size = 0;
            status = KM_YMODEM_ERROR;
          }
        }
      } else {
        packet_size = 0;
      }
    }
  }
  *length = packet_size;
  return status;
}

/**
 * @brief  Receive a file using the ymodem protocol with CRC16.
 * @param header_cb
 * @param packet_cb
 * @return km_ymodem_status_t
 */
km_ymodem_status_t km_ymodem_receive(km_ymodem_header_cb header_cb,
                                     km_ymodem_packet_cb packet_cb,
                                     km_ymodem_footer_cb footer_cb) {
  uint32_t i, packet_length, file_done, errors = 0;
  uint8_t file_name_str[FILE_NAME_LENGTH];
  uint8_t file_size_str[FILE_SIZE_LENGTH];
  uint32_t file_size;
  uint8_t *file_ptr;
  uint32_t packets_received;
  km_ymodem_status_t result = KM_YMODEM_OK;

  session_done = 0;
  session_begin = 0;

  while ((session_done == 0) && (result == KM_YMODEM_OK)) {
    packets_received = 0;
    file_done = 0;
    while ((file_done == 0) && (result == KM_YMODEM_OK)) {
      km_ymodem_status_t stat =
          receive_packet(packet_data, &packet_length, DOWNLOAD_TIMEOUT);
      switch (stat) {
        case KM_YMODEM_OK:
          errors = 0;
          switch (packet_length) {
            case 2: /* Abort by sender */
              km_tty_putc(ACK);
              result = KM_YMODEM_ABORT;
              break;
            case 0: /* End of transmission */
              km_tty_putc(ACK);
              file_done = 1;
              break;
            default: /* Normal packet */
              if (packet_data[PACKET_NUMBER_INDEX] !=
                  (uint8_t)(packets_received & 0x0FF)) {
                km_tty_putc(NAK);
              } else {
                if (packets_received == 0) {
                  /* File name packet */
                  if (packet_data[PACKET_DATA_INDEX] != 0) {
                    /* File name extraction */
                    i = 0;
                    file_ptr = packet_data + PACKET_DATA_INDEX;
                    while ((*file_ptr != 0) && (i < FILE_NAME_LENGTH)) {
                      file_name_str[i++] = *file_ptr++;
                    }
                    file_name_str[i++] = '\0';

                    /* File size extraction */
                    i = 0;
                    file_ptr++;
                    while ((*file_ptr != ' ') && (i < FILE_SIZE_LENGTH)) {
                      file_size_str[i++] = *file_ptr++;
                    }
                    file_size_str[i++] = '\0';
                    file_size = atoi((const char *)file_size_str);

                    // Process the received file header
                    if (header_cb) {
                      int ret = header_cb(file_name_str, file_size);
                      if (ret < 0) {
                        km_tty_putc(CA);
                        km_tty_putc(CA);
                        result = KM_YMODEM_LIMIT;
                      } else {
                        km_tty_putc(ACK);
                        km_tty_putc(CRC16);
                      }
                    }

                  } else { /* File header packet is empty, end session */
                    km_tty_putc(ACK);
                    file_done = 1;
                    session_done = 1;

                    if (footer_cb) {
                      footer_cb();
                    }

                    break;
                  }
                } else { /* Data packet */
                  /* Process the received packet */
                  if (packet_cb) {
                    int ret = packet_cb(packet_data + PACKET_DATA_INDEX,
                                        packet_length);

                    // An error occurred while processing the packet
                    if (ret < 0) {
                      // End session
                      km_tty_putc(CA);
                      km_tty_putc(CA);
                      result = KM_YMODEM_DATA;
                    } else {
                      km_tty_putc(ACK);
                    }
                  }
                }
                packets_received++;
                session_begin = 1;
              }
              break;
          }
          break;
        case KM_YMODEM_ABORT: /* Abort actually */
          km_tty_putc(CA);
          km_tty_putc(CA);
          result = KM_YMODEM_ABORT;
          break;
        default:
          if (session_begin > 0) {
            errors++;
          }
          if (errors > MAX_ERRORS) {
            /* Abort communication */
            km_tty_putc(CA);
            km_tty_putc(CA);
          } else {
            km_tty_putc(CRC16); /* Ask for a packet */
          }
          break;
      }
    }
  }
  return result;
}
