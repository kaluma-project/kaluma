Board
=====

* [Object: board]()
  * [name]()
  * [PIN_NUM]()
  * [PWM_NUM]()
  * [ADC_NUM]()
  * [LED_NUM]()
  * [SWITCH_NUM]()
  * [I2C_NUM]()
  * [SPI_NUM]()
  * [UART_NUM]()
  * [led(num)]()
  * [switch(num)]()
  * [pwm(num)]()
  * [adc(num)]()
  * [i2c(bus[, options])]()
  * [spi(bus[, options])]()
  * [uart(bus[, options])]()


##  board

board specific object

### name

* `{string}`

target board name. ex) 'stm32f4discovery', 'kameleon-core', ...


### PIN_NUM

* `{number}`

### PWM_NUM

* `{number}`

### ADC_NUM

* `{number}`

### LED_NUM

* `{number}`

### SWITCH_NUM

* `{number}`

### I2C_NUM

* `{number}`

### SPI_NUM

* `{number}`

### UART_NUM

* `{number}`

### led(index)

* __`index`__ `{number}`

Return a LED pin number corresponds to the index number.

### switch(index)

* __`index`__ `{number}`

Return a switch pin number corresponds to the index number.

### pwm(index)

* __`index`__ `{number}`

Return a PWM pin number corresponds to the index number.

### adc(index)

* __`index`__ `{number}`

Return a analog (ADC) pin number corresponds to the index number.

### i2c(bus[, options])

* __`bus`__ `{number}`
* __`options`__ `{Object}` Same options object to the `setup()` method.
* Returns: `{I2C}` An initialized I2C instance corresponds to the bus number. Once initialized, the same object will be returned.

Return I2C object initialized with the bus number


### spi(bus[, options])

* __`bus`__ `{number}`
* __`options`__ `{Object}` Same options object to the `setup()` method.
* Returns: `{SPI}` An initialized SPI instance corresponds to the bus number. Once initialized, the same object will be returned.

Return SPI object initialized with the bus number


### uart(bus[, options])

* __`bus`__ `{number}`
* __`options`__ `{Object}` Same options object to the `setup()` method.
* Returns: `{UART}` An initialized UART instance corresponds to the bus number. Once initialized, the same object will be returned.

Return UART object initialized with the bus number
