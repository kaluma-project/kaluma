Board
=====

Common board properties.

* [Object: board]()
  * [name]()
  * [NUM_GPIO]()
  * [NUM_LED]()
  * [NUM_BUTTON]()
  * [NUM_PWM]()
  * [NUM_ADC]()
  * [NUM_I2C]()
  * [NUM_SPI]()
  * [NUM_UART]()
  * [led(pin)]()
  * [button(pin)]()
  * [pwm(pin)]()
  * [adc(pin)]()
  * [i2c(bus[, options])]()
  * [spi(bus[, options])]()
  * [uart(port[, options])]()

Board-specific properties.

* [Board: kameleon-core]()
  * [LED0]()
  * [BTN0]()

##  Object: board

A board specific object.

```js
board.NUM_GPIO = 22
board.NUM_LED = 1
board.NUM_BUTTON = 1
board.NUM_PWM = 6
board.NUM_ADC = 5
board.NUM_I2C = 2
board.NUM_SPI = 2
board.NUM_UART = 2
board.led_pins[]
board.button_pins[]
board.pwm_pins = []
board.adc_pins = []
borad.gpio(pin) -> GPIO()
board.led(pin) -> LED()
board.button(pin) -> Button()
board.pwm(pin) -> PWM()
board.adc(pin) -> ADC()
board.i2c(bus) -> I2C()
board.spi(bus) -> SPI()
board.uart(port) -> UART()
// board-specific
board.LED0 -> LED()
board.BUTTON0 -> Button()
```

### name

* `{string}`

Target board name. ex) `kameleon-core`, ...

### NUM_GPIO

* `{number}` Total number of GPIO.

### NUM_LED

* `{number}` Total number of on-board LEDs.

### NUM_BUTTON

* `{number}` Total number of on-board buttons.

### NUM_PWM

* `{number}` Total number of PWM channels.

### NUM_ADC

* `{number}` Total number of ADCs (Analog-Digital Converter).

### NUM_I2C

* `{number}` Total number of I2C buses.

### NUM_SPI

* `{number}` Total number of SPI buses.

### NUM_UART

* `{number}` Total number of UART ports.

### led(pin)

* __`pin`__ `{number}`
* Returns: `{LED}`

Return a LED object corresponds to the pin number.

### button(pin[, pull[, debounce]])

* __`pin`__ `{number}`
* __`pull`__ `{number}` Optional.
* __`debounce`__ `{number}` Optional.
* Returns: `{Button}`

Return a button object corresponds to the pin number.

### pwm(pin)

* __`pin`__ `{number}`
* Returns: `{PWM}`

Return a PWM object corresponds to the pin number.

### adc(pin)

* __`pin`__ `{number}`
* Returns: `{ADC}`

Return an ADC object corresponds to the pin number.

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

---

## Board: kameleon-core

### LED0

* `{LED}` Readonly.

Indicates the on-board LED (`LED0`).

### BTN0

* `{Button}` Readonly.

Indicates the on-board button (`BTN0`).
