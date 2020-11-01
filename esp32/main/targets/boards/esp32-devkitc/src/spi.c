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

void spi_init()
{
}

void spi_cleanup()
{
  for (int k = 0; k < SPI_NUM; k++) {
    spi_close(k);
  }
}

static spi_device_handle_t handle[SPI_NUM];

int spi_setup(uint8_t bus, spi_mode_t mode, uint32_t baudrate, spi_bitorder_t bitorder)
{
    esp_err_t ret;
    //Configuration for the SPI bus
    spi_bus_config_t buscfg[SPI_NUM];
    spi_host_device_t hostid;
    memset(buscfg, 0, sizeof(spi_bus_config_t) * SPI_NUM);
    if (bus == 0) {
        buscfg[bus].mosi_io_num = GPIO_MOSI_0;
        buscfg[bus].miso_io_num = GPIO_MISO_0;
        buscfg[bus].sclk_io_num = GPIO_SCLK_0;
        buscfg[bus].quadwp_io_num = -1;
        buscfg[bus].quadhd_io_num = -1;
        buscfg[bus].flags = SPICOMMON_BUSFLAG_MASTER;
        hostid = HSPI_HOST;
    } else {
        buscfg[bus].mosi_io_num = GPIO_MOSI_1;
        buscfg[bus].miso_io_num = GPIO_MISO_1;
        buscfg[bus].sclk_io_num = GPIO_SCLK_1;
        buscfg[bus].quadwp_io_num = -1;
        buscfg[bus].quadhd_io_num = -1;
        buscfg[bus].flags = SPICOMMON_BUSFLAG_MASTER;
        hostid = VSPI_HOST;
    }
    spi_device_interface_config_t devcfg[SPI_NUM];
    memset(devcfg, 0, sizeof(spi_device_interface_config_t) * SPI_NUM);
    //Configuration for the SPI device on the other side of the bus
    devcfg[bus].command_bits = 0;
    devcfg[bus].address_bits = 0;
    devcfg[bus].dummy_bits = 0;
    devcfg[bus].clock_speed_hz = baudrate;
    devcfg[bus].duty_cycle_pos = 128;        //50% duty cycle
    devcfg[bus].mode = mode;
    devcfg[bus].spics_io_num = -1;
    devcfg[bus].cs_ena_posttrans = 3;        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
    devcfg[bus].queue_size = 3;
    if (bitorder == SPI_BITORDER_LSB) {
        devcfg[bus].flags = SPI_DEVICE_BIT_LSBFIRST;
    } else {
        devcfg[bus].flags = 0;
    }
    ret=spi_bus_initialize(hostid, &buscfg[bus], bus);
    assert(ret==ESP_OK);
    ret=spi_bus_add_device(hostid, &devcfg[bus], &handle[bus]);
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
    return ret;
}

