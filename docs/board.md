Board
=====

* [Object: board]()
  * [name]()
  * [PIN_COUNT]()
  * [PWM_COUNT]()
  * [ADC_COUNT]()
  * [LED_COUNT]()
  * [BUTTON_COUNT]()
  * [I2C_COUNT]()
  * [SPI_COUNT]()
  * [UART_COUNT]()
  * [led(num)]()
  * [switch(num)]()
  * [pwm(num)]()
  * [adc(num)]()
  * [i2c(bus[, options])]()
  * [spi(bus[, options])]()
  * [uart(port[, options])]()

##  board

board specific object

### name

* `{string}`

target board name. ex) 'stm32f4discovery', 'kameleon-core', ...


### PIN_COUNT

* `{number}` Total number of pins.

### PWM_COUNT

* `{number}` Total number of PWM channels.

### ADC_COUNT

* `{number}` Total number of ADCs.

### LED_COUNT

* `{number}` Total number of LEDs.

### BUTTON_COUNT

* `{number}` Total number of buttons.

### I2C_COUNT

* `{number}` Total number of I2C buses.

### SPI_COUNT

* `{number}` Total number of SPI buses.

### UART_COUNT

* `{number}` Total number of UART ports.

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


### uart(port[, options])

* __`port`__ `{number}`
* __`options`__ `{Object}` Same options object to the `setup()` method.
* Returns: `{UART}` An initialized UART instance corresponds to the port number. Once initialized, the same object will be returned.

Return UART object initialized with the port number
