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
#include <stdlib.h>
#include "uart.h"
#include "ringbuffer.h"

/**
 * Return default UART pins. -1 means there is no default value on that pin.
 */
km_uart_pins_t km_uart_get_default_pins(uint8_t port) {
  km_uart_pins_t pins = {
    .pin_tx = -1,
    .pin_rx = -1,
    .pin_cts = -1,
    .pin_rts = -1,
  };
  return pins;
}

/**
 * Initialize all UART when system started
 */
void km_uart_init() {
}

/**
 * Cleanup all UART when system cleanup
 */
void km_uart_cleanup() {
}

int km_uart_setup(uint8_t port, uint32_t baudrate, uint8_t bits,
    km_uart_parity_type_t parity, uint8_t stop, km_uart_flow_control_t flow,
    size_t buffer_size, km_uart_pins_t pins) {
  return 0;
}

int km_uart_write(uint8_t port, uint8_t *buf, size_t len) {
  return 0;
}

uint32_t km_uart_available(uint8_t port) {
  return 0;
}

uint8_t km_uart_available_at(uint8_t port, uint32_t offset) {
  return 0;
}

uint32_t km_uart_buffer_size(uint8_t port) {
  return 0;
}

uint32_t km_uart_read(uint8_t port, uint8_t *buf, size_t len) {
  return 0;
}

int km_uart_close(uint8_t port) {
  return 0;
}
