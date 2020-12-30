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

#include "esp32_devkitc.h"
#include "spi.h"
#include "gpio.h"
#include <driver/spi_master.h>
#include <string.h>

#define GPIO_MOSI_0 5
#define GPIO_MISO_0 18
#define GPIO_SCLK_0 19
#define GPIO_MOSI_1 27
#define GPIO_MISO_1 26
#define GPIO_SCLK_1 25

#define SPI_PORT0      1 // Bit0
#define SPI_PORT1      2 // Bit1

static spi_device_handle_t handle[SPI_NUM];
static uint8_t spi_status = 0;
void spi_init()
{
}

void spi_cleanup()
{
  for (int k = 0; k < SPI_NUM; k++) {
    if (handle[k] != NULL) {
      spi_close(k);
    }
  }
}

int spi_setup(uint8_t bus, spi_mode_t mode, uint32_t baudrate, spi_bitorder_t bitorder)
{
    esp_err_t ret;
    if (spi_status & (1<<bus))
    {
        return -1; // BUSY
    }
    //Configuration for the SPI bus
    spi_host_device_t hostid;
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI_0,
        .miso_io_num = GPIO_MISO_0,
        .sclk_io_num = GPIO_SCLK_0,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    if (bus == 0) {
        hostid = HSPI_HOST;
        spi_status |= SPI_PORT0;
    } else {
        buscfg.mosi_io_num = GPIO_MOSI_1;
        buscfg.miso_io_num = GPIO_MISO_1;
        buscfg.sclk_io_num = GPIO_SCLK_1;
        hostid = VSPI_HOST;
        spi_status |= SPI_PORT1;
    }
    //Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = baudrate,
        .duty_cycle_pos = 128,        //50% duty cycle
        .mode = mode,
        .spics_io_num = -1,
        .cs_ena_posttrans = 3,        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size = 3,
    };

    if (bitorder == SPI_BITORDER_LSB) {
        devcfg.flags = SPI_DEVICE_BIT_LSBFIRST;
    }
    ret=spi_bus_initialize(hostid, &buscfg, bus);
    assert(ret==ESP_OK);
    ret=spi_bus_add_device(hostid, &devcfg, &handle[bus]);
    assert(ret==ESP_OK);
  return ret;
}

int spi_sendrecv(uint8_t bus, uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=tx_buf;
    t.rx_buffer=rx_buf;
    ret=spi_device_transmit(handle[bus], &t);
    return ret;
}

int spi_send(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=buf;
    ret=spi_device_polling_transmit(handle[bus], &t);
    return ret;
}

int spi_recv(uint8_t bus, uint8_t *buf, size_t len, uint32_t timeout)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.rx_buffer=buf;
    ret=spi_device_transmit(handle[bus], &t);
    return ret;
}

int spi_close(uint8_t bus)
{
    esp_err_t ret;
    ret=spi_bus_remove_device(handle[bus]);
    spi_status &= ~(1<<bus);
    return ret;
}

