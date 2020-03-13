#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#define I2C_MASTER_PORT_NUM 1
#define I2C_MASTER_FREQUENCY 100000
#define I2C_MASTER_SCL_IO 19
#define I2C_MASTER_SDA_IO 18
#define I2C_SLAVE_SCL_IO 26
#define I2C_SLAVE_SDA_IO 25
#define I2C_SLAVE_PORT_NUM 0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define DATA_LENGTH 512
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)

#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT I2C_MASTER_READ

#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0
#define ACK_VAL 0
#define NACK_VAL 1

int _i2c_setup_master(uint8_t bus, uint32_t speed)
{
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = speed;
  i2c_param_config(bus, &conf);
  return i2c_driver_install(bus, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

int _i2c_setup_slave(uint8_t bus, uint8_t address)
{
  i2c_config_t conf;
  conf.mode = I2C_MODE_SLAVE;
  conf.sda_io_num = I2C_SLAVE_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_SLAVE_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.slave.addr_10bit_en = 0;
  conf.slave.slave_addr = address;
  i2c_param_config(bus, &conf);
  return i2c_driver_install(bus, conf.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
int _i2c_master_read_slave(int _i2c_num, int address, uint8_t *data_rd, size_t size, uint32_t timeout)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_port_t i2c_num = _i2c_num;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, timeout / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
int _i2c_master_write_slave(int _i2c_num, int address, uint8_t *data_wr, size_t size, uint32_t timeout)
{
  i2c_port_t i2c_num = _i2c_num;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, timeout / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

int _i2c_slave_read_buffer(int i2c_num, uint8_t *data, size_t max_size, uint32_t timeout)
{
  return i2c_slave_read_buffer(i2c_num, data, max_size, timeout/portTICK_RATE_MS);
}

int _i2c_slave_write_buffer(int i2c_num, const uint8_t *data, int size, uint32_t timeout)
{
  return i2c_slave_write_buffer(i2c_num, (uint8_t*)data, size, timeout/portTICK_RATE_MS);
}
