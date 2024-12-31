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

#include "spi.h"

#include "board.h"
#include "err.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

struct __spi_status_s {
  bool enabled;
} __spi_status[KALUMA_SPI_NUM];

static bool __check_spi_pins(uint8_t bus, km_spi_pins_t pins) {
  if (bus == 0) {
    if ((pins.miso >= 0) && (pins.miso != 0) && (pins.miso != 4) &&
        (pins.miso != 16)) {
      return false;
    }
    if ((pins.mosi >= 0) && (pins.mosi != 3) && (pins.mosi != 7) &&
        (pins.mosi != 19)) {
      return false;
    }
    if ((pins.sck >= 0) && (pins.sck != 2) && (pins.sck != 6) &&
        (pins.sck != 18)) {
      return false;
    }
  } else if (bus == 1) {
    if ((pins.miso >= 0) && (pins.miso != 8) && (pins.miso != 12)) {
      return false;
    }
    if ((pins.mosi >= 0) && (pins.mosi != 11) && (pins.mosi != 15)) {
      return false;
    }
    if ((pins.sck >= 0) && (pins.sck != 10) && (pins.sck != 14)) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

/**
 * Return default SPI pins. -1 means there is no default value on that pin.
 */
km_spi_pins_t km_spi_get_default_pins(uint8_t bus) {
  km_spi_pins_t pins;
  if (bus == 0) {
    pins.miso = 16;
    pins.mosi = 19;
    pins.sck = 18;
  } else if (bus == 1) {
    pins.miso = 12;
    pins.mosi = 11;
    pins.sck = 10;
  } else {
    pins.miso = -1;
    pins.mosi = -1;
    pins.sck = -1;
  }
  return pins;
}

static spi_inst_t *__get_spi_no(uint8_t bus) {
  if (bus == 0) {
    return spi0;
  } else if (bus == 1) {
    return spi1;
  } else {
    return NULL;
  }
}

/**
 * Initialize all SPI when system started
 */
void km_spi_init() {
  for (int i = 0; i < KALUMA_SPI_NUM; i++) {
    __spi_status[i].enabled = false;
  }
}

/**
 * Cleanup all SPI when system cleanup
 */
void km_spi_cleanup() {
  spi_deinit(spi0);
  spi_deinit(spi1);
  km_spi_init();
}

/** SPI Setup
 */
int km_spi_setup(uint8_t bus, km_spi_mode_t mode, uint32_t baudrate,
                 km_spi_bitorder_t bitorder, km_spi_pins_t pins,
                 km_spi_pullup_t data_pullup) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled) ||
      (__check_spi_pins(bus, pins) == false)) {
    return EDEVINIT;
  }
  spi_cpol_t pol = SPI_CPOL_0;
  spi_cpha_t pha = SPI_CPHA_0;
  spi_order_t order;
  switch (mode) {
    case KM_SPI_MODE_0:
      pol = SPI_CPOL_0;
      pha = SPI_CPHA_0;
      break;
    case KM_SPI_MODE_1:
      pol = SPI_CPOL_0;
      pha = SPI_CPHA_1;
      break;
    case KM_SPI_MODE_2:
      pol = SPI_CPOL_1;
      pha = SPI_CPHA_0;
      break;
    case KM_SPI_MODE_3:
      pol = SPI_CPOL_1;
      pha = SPI_CPHA_1;
      break;
  }
  if (bitorder == KM_SPI_BITORDER_MSB) {
    order = SPI_MSB_FIRST;
  } else {
    order = SPI_LSB_FIRST;
  }
  spi_init(spi, baudrate);
  spi_set_format(spi, 8, pol, pha, order);
  if (pins.miso >= 0) {
    gpio_set_function(pins.miso, GPIO_FUNC_SPI);
    if (data_pullup & KM_SPI_MISO_PULLUP) {
      gpio_pull_up(pins.miso);
    } else {
      gpio_set_pulls(pins.miso, false, false);
    }
  }
  if (pins.mosi >= 0) {
    gpio_set_function(pins.mosi, GPIO_FUNC_SPI);
    if (data_pullup & KM_SPI_MOSI_PULLUP) {
      gpio_pull_up(pins.mosi);
    } else {
      gpio_set_pulls(pins.mosi, false, false);
    }
  }
  if (pins.sck >= 0) {
    gpio_set_function(pins.sck, GPIO_FUNC_SPI);
    gpio_set_pulls(pins.sck, false, false);
  }
  __spi_status[bus].enabled = true;
  return 0;
}

int km_spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, size_t len,
                    uint32_t timeout) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled == false)) {
    return EDEVREAD;
  }
  (void)timeout;  // timeout is not supported.
  return spi_write_read_blocking(spi, tx_buf, rx_buf, len);
}

int km_spi_send(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled == false)) {
    return EDEVWRITE;
  }
  (void)timeout;  // timeout is not supported.
  return spi_write_blocking(spi, buf, len);
}

int km_spi_recv(uint8_t bus, uint8_t send_byte, uint8_t *buf, size_t len,
                uint32_t timeout) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled == false)) {
    return EDEVREAD;
  }
  (void)timeout;  // timeout is not supported.
  return spi_read_blocking(spi, send_byte, buf, len);
}

int km_set_spi_baudrate(uint8_t bus, uint32_t baudrate) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled == false)) {
    return ENODEV;
  }
  spi_set_baudrate(spi, baudrate);
  return 0;
}

int km_spi_close(uint8_t bus) {
  spi_inst_t *spi = __get_spi_no(bus);
  if ((spi == NULL) || (__spi_status[bus].enabled == false)) {
    return EDEVINIT;
  }
  spi_deinit(spi);
  __spi_status[bus].enabled = false;
  return 0;
}
