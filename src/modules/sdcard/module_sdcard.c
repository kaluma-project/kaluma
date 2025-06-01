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

#include "module_sdcard.h"

#include <stdlib.h>

#include "board.h"
#include "err.h"
#include "gpio.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"
#include "sdcard_magic_strings.h"
#include "spi.h"
#include "system.h"

#define SD_BLOCK 512
#define SD_CMD0 0   /* GO_IDLE_STATE */
#define SD_CMD1 1   /* SEND_OP_COND */
#define SD_CMD6 6   /* SWITCH_FUNC */
#define SD_CMD8 8   /* SEND_IF_COND */
#define SD_CMD9 9   /* SEND_CSD */
#define SD_CMD10 10 /* SEND_CID */
#define SD_CMD12 12 /* STOP_TRANSMISSION */
#define SD_CMD13 13 /* SEND_STATUS */
#define SD_CMD16 16 /* SET_BLOCKLEN */
#define SD_CMD17 17 /* READ_SINGLE_BLOCK */
#define SD_CMD18 18 /* READ_MULTIPLE_BLOCK */
#define SD_CMD23 23 /* SET_BLOCK_COUNT */
#define SD_CMD24 24 /* WRITE_BLOCK */
#define SD_CMD25 25 /* WRITE_MULTIPLE_BLOCK */
#define SD_CMD32 32 /* ERASE_ER_BLK_START */
#define SD_CMD33 33 /* ERASE_ER_BLK_END */
#define SD_CMD38 38 /* ERASE */
#define SD_CMD42 42 /* LOCK_UNLOCK */
#define SD_CMD55 55 /* APP_CMD */
#define SD_CMD56 56 /* GEN_CMD */
#define SD_CMD58 58 /* READ_OCR */
#define SD_CMD59 59 /* CRC_ON_OFF */

#define ACMD_FLAG 0x80             /* ACMD BIT */
#define SD_ACMD13 (ACMD_FLAG + 13) /* APP_SD_STATUS */
#define SD_ACMD22 (ACMD_FLAG + 22) /* APP_SEND_NUM_WR_BLOCKS */
#define SD_ACMD23 (ACMD_FLAG + 23) /* APP_SET_WR_BLK_ERASE_COUNT */
#define SD_ACMD41 (ACMD_FLAG + 41) /* APP_SEND_OP_COND */
#define SD_ACMD42 (ACMD_FLAG + 42) /* APP_SET_CLR_CARD_DETECT */
#define SD_ACMD51 (ACMD_FLAG + 51) /* APP_SEND_SCR */

#define R1_IN_IDLE 0x01
#define R1_ERASE_RESET 0x02
#define R1_ILLEGAL_CMD 0x04
#define R1_CRC_ERROR 0x08
#define R1_ERASE_SEQ_ERROR 0x10
#define R1_ADDRESS_ERROR 0x20
#define R1_PARAMETER_ERROR 0x40
#define R1_ERRORS                                                        \
  (R1_ERASE_RESET | R1_ILLEGAL_CMD | R1_CRC_ERROR | R1_ERASE_SEQ_ERROR | \
   R1_ADDRESS_ERROR | R1_PARAMETER_ERROR)

#define SD_TYPE_NONE 0
#define SD_TYPE_MMC 0x01
#define SD_TYPE_SDSC1 0x02
#define SD_TYPE_SDSC2 0x04
#define SD_TYPE_SDHC 0x08
#define SD_TYPE_SDSC (SD_TYPE_SDSC1 | SD_TYPE_SDSC2)

#define SD_STATUS_INIT 0x01

#define SLOW_BAUDRATE 400000   /* 400 kHZ */
#define FAST_BAUDRATE 25000000 /* 25 MHZ */

#define CS_HIGH km_gpio_write(__sdcard_handle.cs_pin, KM_GPIO_HIGH)
#define CS_LOW km_gpio_write(__sdcard_handle.cs_pin, KM_GPIO_LOW)

typedef struct __sdcard_handle_s {
  uint8_t bus;
  uint8_t cs_pin;
  uint8_t status;
  uint8_t sd_type;
  int size;
  int count;
} __sdcard_handle_t;

static __sdcard_handle_t __sdcard_handle;

static int __wait_for_ready(uint32_t timeout_ms) {
  uint8_t send = 0xFF;
  uint8_t receive;
  uint64_t start_ms = km_gettime();
  do {
    km_spi_sendrecv(__sdcard_handle.bus, &send, &receive, 1, 100);
  } while ((receive != 0xFF) && (km_gettime() < start_ms + timeout_ms));
  if (receive == 0xFF) {
    return 0;
  }
  return ETIMEDOUT;
}

static void __deselect(void) {
  uint8_t send = 0xFF;
  CS_HIGH;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);  // dummy clock
}

static int __select(void) {
  uint8_t send = 0xFF;
  CS_LOW;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);  // dummy clock
  if (__wait_for_ready(500) < 0) {
    __deselect();
    return ETIMEDOUT;
  }
  return 0;
}

/**
 * @brief send sdcard command
 *
 * @param command command
 * @param arg argument
 * @param response response of the command (output)
 * @return uint8_t 1 is R1, uint8_t2 is R2. minus value if the function return
 * error.
 */
static int __send_command(uint8_t command, uint32_t arg, uint8_t crc) {
  uint8_t res = 0;
  uint8_t res2 = 0;
  int ret;
  uint8_t retry = 10;
  uint8_t command_buffer[6];
  if (command & ACMD_FLAG) {
    res = __send_command(SD_CMD55, 0, 0x00);
    if (res & R1_ERRORS) {
      return res;
    }
    command &= ~ACMD_FLAG;
  }
  if (command != SD_CMD12) {
    __deselect();
    if (__select() < 0) {
      return ETIMEDOUT;
    }
  }
  command_buffer[0] = (0x40 | command);
  command_buffer[1] = ((arg >> 24) & 0xFF);
  command_buffer[2] = ((arg >> 16) & 0xFF);
  command_buffer[3] = ((arg >> 8) & 0xFF);
  command_buffer[4] = (arg & 0xFF);
  command_buffer[5] = (crc | 0x01);  // Dummy CRC because CRC is not checked.
  km_spi_send(__sdcard_handle.bus, command_buffer, 6, 600);
  // receive response
  if ((command == SD_CMD12) || (command == SD_CMD38)) {  // R1b response
    do {
      km_spi_recv(__sdcard_handle.bus, 0xFF, &res, 1, 100);
    } while ((res > 0) && --retry);
  } else {
    do {
      km_spi_recv(__sdcard_handle.bus, 0xFF, &res, 1, 100);
    } while ((res & 0x80) && --retry);
  }
  if (command == SD_CMD13) {  // R2 response
    km_spi_recv(__sdcard_handle.bus, 0xFF, &res2, 1, 100);
  }
  ret = ((res2 << 8) | res);
  return ret;
}

static int __receive_datablock(uint8_t *buff, unsigned int length) {
  int ret = 0;
  uint8_t send = 0xFF;
  uint8_t tocken;
  const uint32_t timeout_ms = 200;
  uint64_t start_ms = km_gettime();
  do {
    km_spi_recv(__sdcard_handle.bus, 0xFF, &tocken, 1, 10);
  } while (tocken == 0xFF && km_gettime() < start_ms + timeout_ms);
  if (tocken == 0xFE) {
    km_spi_recv(__sdcard_handle.bus, 0xFF, buff, length, length * 10);
    km_spi_recv(__sdcard_handle.bus, 0xFF, &tocken, 1, 10);  // CRC
    km_spi_recv(__sdcard_handle.bus, 0xFF, &tocken, 1, 10);  // CRC
  } else {
    ret = ETIMEDOUT;
  }
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  CS_HIGH;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  return ret;
}

static int __send_datablock(uint8_t *buff, unsigned int length) {
  int ret = 0;
  uint8_t send = 0xFF;
  uint8_t tocken = 0xFE;
  km_spi_send(__sdcard_handle.bus, &tocken, 1, 100);  // Send start token
  km_spi_send(__sdcard_handle.bus, buff, length,
              length * 10);  // Send start token
  const uint32_t timeout_ms = 300;
  uint64_t start_ms = km_gettime();
  do {
    km_spi_recv(__sdcard_handle.bus, 0xFF, &tocken, 1, 10);
  } while (tocken == 0xFF && km_gettime() < start_ms + timeout_ms);
  if ((tocken & 0x1F) == 0x05) {  // Dsta accepted
    start_ms = km_gettime();
    do {
      km_spi_recv(__sdcard_handle.bus, 0xFF, &tocken, 1, 10);
    } while (tocken == 0x00 && km_gettime() < start_ms + timeout_ms);
  } else {
    ret = ETIMEDOUT;
  }
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  CS_HIGH;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  return ret;
}

static int __erase_datablock(uint32_t start, uint32_t end) {
  int ret = 0;
  uint8_t send = 0xFF;
  if ((__send_command(SD_CMD32, start, 0x00) != 0) ||
      (__send_command(SD_CMD33, end, 0x00) != 0) ||
      (__send_command(SD_CMD38, 0, 0x00) != 0)) {
    ret = ETIMEDOUT;
  }
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  CS_HIGH;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  return ret;
}

int __sdcard_init(void) {
  uint8_t send = 0xFF;
  uint64_t start_ms;
  const uint16_t timeout_ms = 1000;
  uint8_t resp[16];
  int res;
  if ((__sdcard_handle.status & SD_STATUS_INIT)) {
    return EALREADY;
  }
  CS_HIGH;
  km_delay(1);                                        // 1ms delay
  for (int i = 0; i < 10; i++) {                      // 80 cycle clock
    km_spi_send(__sdcard_handle.bus, &send, 1, 100);  // dummy clock
  }
  CS_LOW;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);  // dummy clock
  if (__send_command(SD_CMD0, 0, 0x94) == R1_IN_IDLE) {
    start_ms = km_gettime();
    if (__send_command(SD_CMD8, 0x1AA, 0x86) == R1_IN_IDLE) {
      km_spi_recv(__sdcard_handle.bus, 0xFF, resp, 4, 400);
      if ((resp[2] == 0x01) && (resp[3] == 0xAA)) {
        do {
          res = __send_command(SD_ACMD41, 0x40000000, 0x00);
        } while ((res != 0) && (km_gettime() < start_ms + timeout_ms));
        if (res == 0) {
          __sdcard_handle.sd_type = SD_TYPE_SDSC2;
          if (__send_command(SD_CMD58, 0, 0x00) == 0) {
            km_spi_recv(__sdcard_handle.bus, 0xFF, resp, 4, 400);
            if (resp[0] & 0x40) {
              __sdcard_handle.sd_type |= SD_TYPE_SDHC;
            }
          }
        }
      }
    } else {
      uint8_t next_cmd;
      if (__send_command(SD_ACMD41, 0, 0x00) <= R1_IN_IDLE) {
        __sdcard_handle.sd_type = SD_TYPE_SDSC1;
        next_cmd = SD_ACMD41;
      } else {
        __sdcard_handle.sd_type = SD_TYPE_MMC;
        next_cmd = SD_CMD1;
      }
      do {
        res = __send_command(next_cmd, 0, 0x00);
      } while ((res != 0) && (km_gettime() < start_ms + timeout_ms));
      if (res == 0) {
        res = __send_command(SD_CMD16, SD_BLOCK, 0x00);
        if (res != 0) {
          __sdcard_handle.sd_type = SD_TYPE_NONE;
        }
      } else {
        __sdcard_handle.sd_type = SD_TYPE_NONE;
      }
    }
  }
  if (__sdcard_handle.sd_type != SD_TYPE_NONE) {
    res = __send_command(SD_CMD9, 0, 0x00);
    if (res != 0) {
      __sdcard_handle.sd_type = SD_TYPE_NONE;
    } else {
      uint8_t data = 0;
      uint8_t retry = 10;
      do {
        km_spi_recv(__sdcard_handle.bus, 0xFF, &data, 1, 100);
      } while ((data != 0xFE) && --retry);
      km_spi_recv(__sdcard_handle.bus, 0xFF, resp, 16, 1600);
      int bl_len = (resp[5] & 0x0F);
      __sdcard_handle.size = (1 << bl_len);
      if (resp[0] == 0x00) {
        int c_size =
            (((resp[6] & 0x3) << 10) | (resp[7] << 2) | (resp[8] >> 6));
        int c_size_multi = (((resp[9] & 0x3) << 1) | (resp[10] >> 7));
        __sdcard_handle.count = (c_size + 1) * (1 << (c_size_multi + 2));
      } else if (resp[0] == 0x40) {
        int c_size = (((resp[7] & 0x3F) << 16) | (resp[8] << 8) | resp[9]);
        c_size++;
        __sdcard_handle.count = (c_size << 10);
      }
    }
  }
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);
  CS_HIGH;
  km_spi_send(__sdcard_handle.bus, &send, 1, 100);

  if (__sdcard_handle.sd_type != SD_TYPE_NONE) {
    __sdcard_handle.status |= SD_STATUS_INIT;
    km_set_spi_baudrate(__sdcard_handle.bus, FAST_BAUDRATE);
  }
  return (int)__sdcard_handle.status;
}

static void init_sd(void) {
  __sdcard_handle.bus = 0;
  __sdcard_handle.cs_pin = 0;
  __sdcard_handle.sd_type = SD_TYPE_NONE;
  __sdcard_handle.status = 0;
  __sdcard_handle.count = 0;
  __sdcard_handle.size = 0;
}
/**
 * Sdcard (block device) constructor
 * args:
 *   bus {number} SPI bus
 *   options (number) SPI options
 */
JERRYXX_FUN(sdcard_ctor_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "bus");
  JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options");

  // read parameters
  uint8_t bus = (uint8_t)JERRYXX_GET_ARG_NUMBER(0);
  uint8_t mode = KM_SPI_MODE_0;
  uint32_t baudrate = SLOW_BAUDRATE;
  uint8_t bitorder = KM_SPI_BITORDER_MSB;
  km_spi_pins_t def_pins = km_spi_get_default_pins(bus);
  km_spi_pins_t pins = {
      .miso = def_pins.miso,
      .mosi = def_pins.mosi,
      .sck = def_pins.sck,
  };

  uint8_t cs_pin = 0;
  if (JERRYXX_HAS_ARG(1)) {
    jerry_value_t options = JERRYXX_GET_ARG(1);
    cs_pin =
        (int8_t)jerryxx_get_property_number(options, MSTR_SDCARD_SPI_CS, 0);
    pins.miso = (int8_t)jerryxx_get_property_number(
        options, MSTR_SDCARD_SPI_MISO, def_pins.miso);
    pins.mosi = (int8_t)jerryxx_get_property_number(
        options, MSTR_SDCARD_SPI_MOSI, def_pins.mosi);
    pins.sck = (int8_t)jerryxx_get_property_number(options, MSTR_SDCARD_SPI_SCK,
                                                   def_pins.sck);
  }
  // initialize the bus
  int ret = km_gpio_set_io_mode(cs_pin, KM_GPIO_IO_MODE_OUTPUT);
  if (ret < 0) {
    return jerry_error_sz(
        JERRY_ERROR_COMMON,
        "SD Card CS pin setup error.");
  }
  CS_HIGH;
  ret = km_spi_setup(bus, (km_spi_mode_t)mode, baudrate,
                     (km_spi_bitorder_t)bitorder, pins, KM_SPI_MISO_PULLUP);
  if (ret < 0) {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "SD Card SPI setup error.");
  }
  init_sd();
  __sdcard_handle.bus = bus;
  __sdcard_handle.cs_pin = cs_pin;
  return jerry_undefined();
}

/**
 * Sdcard.prototype.read()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(sdcard_read_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "block")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  int block = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  // int offset = JERRYXX_GET_ARG_NUMBER_OPT(2, 0);

  // get buffer pointer
  jerry_length_t buffer_length = 0;
  jerry_length_t buffer_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  if (!(__sdcard_handle.status & SD_STATUS_INIT)) {
    return jerry_error_sz(
        JERRY_ERROR_COMMON, "SDCard is not initialized.");
  }
  if (__send_command(SD_CMD17, block, 0x00) != 0xFF) {
    if (__receive_datablock(buffer_pointer, __sdcard_handle.size) < 0) {
      return jerry_error_sz(JERRY_ERROR_COMMON,
                                "SDCard read error.");
    }
  }
  return jerry_undefined();
}

/**
 * Sdcard.prototype.write()
 * args:
 *   block {number}
 *   buffer {Uint8Array}
 *   offset {number}
 */
JERRYXX_FUN(sdcard_write_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "block")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  int block = JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  // int offset = JERRYXX_GET_ARG_NUMBER_OPT(2, 0);

  // get buffer pointer
  jerry_length_t buffer_length = 0;
  jerry_length_t buffer_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buffer_offset, &buffer_length);
  uint8_t *buffer_pointer = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  if (!(__sdcard_handle.status & SD_STATUS_INIT)) {
    return jerry_error_sz(
        JERRY_ERROR_COMMON, "SDCard is not initialized.");
  }
  if ((__send_command(SD_CMD24, block, 0x00) != 0x00) ||
      (__send_datablock(buffer_pointer, __sdcard_handle.size) < 0)) {
    return jerry_error_sz(JERRY_ERROR_COMMON,
                              "SDCard write error.");
  }
  return jerry_undefined();
}

/**
 * Sdcard.prototype.ioctl()
 * args:
 *   op {number}
 *   arg {number}
 */
JERRYXX_FUN(sdcard_ioctl_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "op")
  JERRYXX_CHECK_ARG_NUMBER_OPT(1, "arg")
  int op = JERRYXX_GET_ARG_NUMBER(0);
  int arg = JERRYXX_GET_ARG_NUMBER_OPT(1, 0);
  int ret;

  switch (op) {
    case 1:  // init
      ret = __sdcard_init();
      if (ret <= 0) {
        return jerry_error_sz(JERRY_ERROR_COMMON,
                                  "SD Card init error.");
      }
      return jerry_number(ret);
    case 2:  // shutdown
      init_sd();
      return jerry_number(0);
    case 3:  // sync
      return jerry_number(0);
    case 4:  // block count
      return jerry_number(__sdcard_handle.count);
    case 5:  // block size
      return jerry_number(__sdcard_handle.size);
    case 6:  // erase block
      if (!(__sdcard_handle.status & SD_STATUS_INIT)) {
        return jerry_error_sz(
            JERRY_ERROR_COMMON,
            "SDCard is not initialized.");
      } else {
        if (__erase_datablock(arg, arg) < 0) {
          return jerry_error_sz(
              JERRY_ERROR_COMMON, "SDCard earse error.");
        }
      }
      return jerry_number(0);
    case 7:  // buffer size
      return jerry_number(__sdcard_handle.size);
    default:
      return jerry_error_sz(JERRY_ERROR_COMMON,
                                "Unknown operation.");
  }
}

/**
 * Initialize 'sdcard' module and return exports
 */
jerry_value_t module_sdcard_init() {
  /* Sdcard class */
  jerry_value_t sdcard_ctor = jerry_function_external(sdcard_ctor_fn);
  jerry_value_t sdcard_prototype = jerry_object();
  jerryxx_set_property(sdcard_ctor, MSTR_PROTOTYPE, sdcard_prototype);
  jerryxx_set_property_function(sdcard_prototype, MSTR_SDCARD_READ,
                                sdcard_read_fn);
  jerryxx_set_property_function(sdcard_prototype, MSTR_SDCARD_WRITE,
                                sdcard_write_fn);
  jerryxx_set_property_function(sdcard_prototype, MSTR_SDCARD_IOCTL,
                                sdcard_ioctl_fn);
  jerry_value_free(sdcard_prototype);

  /* sdcard module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_SDCARD_SDCARD, sdcard_ctor);
  jerry_value_free(sdcard_ctor);
  return exports;
}
