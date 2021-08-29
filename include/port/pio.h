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

#ifndef __KM_PIO_H
#define __KM_PIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KM_PIO_ERROR -1

#define KM_PIO_FIFO_LOW 0
#define KM_PIO_FIFO_HIGH 1

#define KM_PIO_NO_SM 4

#define KM_PIO_PORT_DISABLE 0x00
#define KM_PIO_PORT_ENABLE 0x01

#define KM_PIO_SM0_EN 0x10
#define KM_PIO_SM1_EN 0x20
#define KM_PIO_SM2_EN 0x40
#define KM_PIO_SM3_EN 0x80

/**
 * Init a PIO module.
 *
 */
void km_pio_init(void);

/**
 * reset a PIO module.
 *
 */
void km_pio_cleanup(void);

/**
 * Init a PIO port.
 *
 * @param port port number of PIO block
 * @param code code array for PIO
 * @param code_length Length of the code.
 * @return 0 or Positive number if successfully setup, negative otherwise.
 */
int km_pio_port_init(uint8_t port, uint16_t *code, uint8_t code_length);

/**
 * Setup a PIO sm.
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_sm_setup(uint8_t port, uint8_t sm);

/**
 * Setup a PIO sm output pins.
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @param pin_out output base pin number for the PIO
 * @param pin_out_cnt output consecutive pin counts
 * @return Positive number if successfully setup output pins, negative
 * otherwise.
 */
int km_pio_sm_set_out(uint8_t port, uint8_t sm, uint8_t pin_out,
                      uint8_t pin_out_cnt);

/**
 * Setup a PIO sm input pins.
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @param pin_in input base pin number for the PIO
 * @param pin_in_cnt input consecutive pin count
 * @return Positive number if successfully setup input pins, negative otherwise.
 */
int km_pio_sm_set_in(uint8_t port, uint8_t sm, uint8_t pin_in,
                     uint8_t pin_in_cnt);

/**
 * initialize a PIO sm.
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @return Positive number if successfully initialized, negative otherwise.
 */
int km_pio_sm_init(uint8_t port, uint8_t sm);

/**
 * Enable/Disable a PIO sm.
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @param en true is enable, false is diable
 * @return Positive number if successfully enable or disabled, negative
 * otherwise.
 */
int km_pio_sm_enable(uint8_t port, uint8_t sm, bool en);

/**
 * Close the PIO port
 *
 * @param port port number of PIO block
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_close(uint8_t port);

/**
 * Put data into the PIO FIFO
 *
 * @param port port number of PIO block
 * @param data 32bit data value
 * @return Positive number if successfully setup, negative otherwise.
 */
int km_pio_put_fifo(uint8_t port, uint8_t sm, uint32_t data);

/**
 *  GEt data from the PIO FIFO
 *
 * @param port port number of PIO block
 * @param sm state machine
 * @param err error code, O if there's no error.
 * @return 32bit data value
 */
uint32_t km_pio_get_fifo(uint8_t port, uint8_t sm, int8_t *err);
#endif /* __KM_PIO_H */
