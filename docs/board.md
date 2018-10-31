Board
=====

* [Object: board]()
  * [name]()
  * [PIN_NUM]()
  * [LED_NUM]()
  * [BUTTON_NUM]()
  * [PWM_NUM]()
  * [ADC_NUM]()
  * [I2C_NUM]()
  * [SPI_NUM]()
  * [UART_NUM]()
  * [led(num)]()
  * [button(num)]()
  * [pwm(num)]()
  * [adc(num)]()
  * [i2c(bus[, options])]()
  * [spi(bus[, options])]()
  * [uart(port[, options])]()

##  board

board specific object

```js
board.GPIO_NUM = 22
board.LED_NUM = 1
board.BUTTON_NUM = 1
board.PWM_NUM = 6
board.ADC_NUM = 5
board.I2C_NUM = 2
board.SPI_NUM = 2
board.UART_NUM = 2
board.led_pins[]
board.button_pins[]
board.pwm_pins[]
board.adc_pins[]
borad.gpio() -> GPIO()
board.led() -> LED()
board.button() -> Button()
board.pwm() -> PWM()
board.adc() -> ADC()
board.i2c() -> I2C()
board.spi() -> SPI()
board.uart() -> UART()
```

### name

* `{string}`

target board name. ex) 'stm32f4discovery', 'kameleon-core', ...


### PIN_NUM

* `{number}` Total number of pins.

### PWM_NUM

* `{number}` Total number of PWM channels.

### ADC_NUM

* `{number}` Total number of ADCs.

### LED_NUM

* `{number}` Total number of LEDs.

### BUTTON_NUM

* `{number}` Total number of buttons.

### I2C_NUM

* `{number}` Total number of I2C buses.

### SPI_NUM

* `{number}` Total number of SPI buses.

### UART_NUM

* `{number}` Total number of UART ports.

### led(index)

* __`index`__ `{number}`

Return a LED pin number corresponds to the index number.

### button(index)

* __`index`__ `{number}`

Return a button pin number corresponds to the index number.

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
