#include <stdlib.h>
#include "tty.h"
#include "ymodem.h"

uint8_t packet_data[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint32_t session_done, session_begin;

static ymodem_status_t receive_packet(uint8_t *data, uint32_t *length, uint32_t timeout);
static uint16_t update_crc16(uint16_t crc_in, uint8_t byte);
static uint16_t calc_crc16(const uint8_t* p_data, uint32_t size);

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
    if (in & 0x100)
      ++crc;
    if (crc & 0x10000)
      crc ^= 0x1021;
  } while (!(in & 0x10000));
  return crc & 0xffffu;
}

/**
 * @brief  Calculate CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @return None
 */
static uint16_t calc_crc16(const uint8_t* data, uint32_t size) {
  uint32_t crc = 0;
  const uint8_t* dataEnd = data + size;
  while(data < dataEnd) {
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
 * @return YMODEM_OK: normally return
 *         YMODEM_ABORT: aborted by user
 */
static ymodem_status_t receive_packet(uint8_t *data, uint32_t *length, uint32_t timeout) {
  uint32_t crc;
  uint32_t packet_size = 0;
  ymodem_status_t status;
  uint8_t ch;
  uint32_t ret;

  *length = 0;
  ret = tty_read_sync(&ch, 1, timeout);
  if (ret > 0) {
    status = YMODEM_OK;
  } else {
    status = YMODEM_TIMEOUT;
  }

  if (status == YMODEM_OK) {
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
        ret = tty_read_sync(&ch, 1, timeout);
        if ((ret > 0) && (ch == CA)) {
          packet_size = 2;
        } else {
          status = YMODEM_ERROR;
        }
        break;
      case ABORT1:
      case ABORT2:
        status = YMODEM_ABORT;
        break;
      default:
        status = YMODEM_ERROR;
        break;
    }
    *data = ch;

    if (packet_size >= PACKET_SIZE) {
      ret = tty_read_sync(&data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);
      if (ret > 0) {
        status = YMODEM_OK;
      } else {
        status = YMODEM_TIMEOUT;
      }

      /* Simple packet sanity check */
      if (status == YMODEM_OK ) {
        if (data[PACKET_NUMBER_INDEX] != ((data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
          packet_size = 0;
          status = YMODEM_ERROR;
        } else {
          /* Check packet CRC */
          crc = data[packet_size + PACKET_DATA_INDEX] << 8;
          crc += data[packet_size + PACKET_DATA_INDEX + 1];
          if (calc_crc16(&data[PACKET_DATA_INDEX], packet_size) != crc ) {
            packet_size = 0;
            status = YMODEM_ERROR;
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
 * @return ymodem_status_t
 */
ymodem_status_t ymodem_receive(ymodem_header_cb header_cb,
    ymodem_packet_cb packet_cb, ymodem_footer_cb footer_cb) {
  uint32_t i, packet_length, file_done, errors = 0;
  uint8_t file_name_str[FILE_NAME_LENGTH];
  uint8_t file_size_str[FILE_SIZE_LENGTH];
  uint32_t file_size;
  uint8_t *file_ptr;
  uint32_t packets_received;
  ymodem_status_t result = YMODEM_OK;

  session_done = 0;
  session_begin = 0;

  while ((session_done == 0) && (result == YMODEM_OK)) {
    packets_received = 0;
    file_done = 0;
    while ((file_done == 0) && (result == YMODEM_OK)) {
      ymodem_status_t stat = receive_packet(packet_data, &packet_length, DOWNLOAD_TIMEOUT);
      switch (stat) {
        case YMODEM_OK:
          errors = 0;
          switch (packet_length) {
            case 2: /* Abort by sender */
              tty_putc(ACK);
              result = YMODEM_ABORT;
              break;
            case 0: /* End of transmission */
              tty_putc(ACK);
              file_done = 1;
              break;
            default: /* Normal packet */
              if (packet_data[PACKET_NUMBER_INDEX] != (uint8_t)(packets_received & 0x0FF)) {
                tty_putc(NAK);
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
                    file_size = atoi((const char*)file_size_str);

                    // Process the received file header
                    if (header_cb) {
                      int ret = header_cb(file_name_str, file_size);
                      if (ret < 0) {
                        tty_putc(CA);
                        tty_putc(CA);
                        result = YMODEM_LIMIT;
                      } else {
                        tty_putc(ACK);
                        tty_putc(CRC16);
                      }
                    }

                  } else { /* File header packet is empty, end session */
                    tty_putc(ACK);
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
                    int ret = packet_cb(packet_data + PACKET_DATA_INDEX, packet_length);

                    // An error occurred while processing the packet
                    if (ret < 0) {
                      // End session
                      tty_putc(CA);
                      tty_putc(CA);
                      result = YMODEM_DATA;
                    } else {
                      tty_putc(ACK);
                    }
                  }
                }
                packets_received ++;
                session_begin = 1;
              }
              break;
          }
          break;
        case YMODEM_ABORT: /* Abort actually */
          tty_putc(CA);
          tty_putc(CA);
          result = YMODEM_ABORT;
          break;
        default:
          if (session_begin > 0) {
            errors ++;
          }
          if (errors > MAX_ERRORS) {
            /* Abort communication */
            tty_putc(CA);
            tty_putc(CA);
          } else {
            tty_putc(CRC16); /* Ask for a packet */
          }
          break;
      }
    }
  }
  return result;
}
