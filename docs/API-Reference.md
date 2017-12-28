API REFERENCE
=============

## Globals

### Constants

* `LOW` = 0
* `HIGH` = 1
* `INPUT` = 0
* `OUTPUT` = 1
* `FALLING` = 0
* `RISING` = 1
* `CHANGE` = 2

### Functions

Digital I/O

* `pinMode(pin, mode)`
* `digitalRead(pin) -> number`
* `digitalWrite(pin, value)`
* `digitalToggle(pin)`

Analog I/O

* `analogRead(pin)`
  * `pin {number}`
  * `@return {number}` Return value in between 0.0 ~ 1.0 (when returns int value, it depends on hardware: 0~1023, 0~4095, ...)
* `analogWrite(pin, value)` -- (How frequency will be set?)
  * `pin {number}`
  * `value {number}` -- Duty cycle between 0 and 1.

Additional I/O

* `tone(pin, frequency, duration, ?duty)`
* `noTone(pin)`
* `setWatch(callback, pin, ?mode=CHANGE, ?debounce=0) -> id` _(from gpio)_
  * `callback {Function: () -> undefined}`
  * `pin {number}`
  * `?mode=CHANGE {number}` -- RISING | FALLING | CHANGE
  * `?debounce=0 {number}`
  * Returns an `id {number}` for watcher.
* `clearWatch(id)`

Time

* `delay(msec)`
* `millis()`
* `setTimeout(callback, msec) -> id`
* `setInterval(callback, msec) -> id`
* `clearTimeout(id)`
* `clearInterval(id)`

Etc.

* `require(module)`
* `print(value)` -- equivalent to console.log

### Objects

* `global`
* `console`
  * `log()`
  * ~~`warn()`~~
  * `error()`
* `process`
  * `builtin_modules` - _array of builtin module names_
  * `binding(native_module_name)` - _load native modules_
    * `<native_module_name>` - _binding has native module names as properties_
  * `arch`-- ex) 'arm', 'x64', ...
  * `platform` -- ex) 'linux', 'darwin', 'unknown', ...
  * `version` -- semver format. ex) '0.1.0', ...
  * (event) `onUncaughtException`
* `board` -- board specific object
  * `name` -- target board name. ex) 'stm32f4discovery', 'kameleon-core', ...
  * `PIN_NUM`
  * `PWM_NUM`
  * `ADC_NUM`
  * `LED_NUM`
  * `SWITCH_NUM`
  * `I2C_NUM`
  * `SPI_NUM`
  * `UART_NUM`
  * `led(num)` -- return corresponding pin number
  * `switch(num)` -- return corresponding pin number
  * `pwm(num)` -- return corresponding pin number
  * `adc(num)` -- return corresponding pin number
  * `i2c(bus, options)` -- return I2C object initialized with the bus number
  * `spi(bus, options)` -- return SPI object initialized with the bus number
  * `uart(bus, options)` -- return UART object initialized with the bus number

## Builtin Modules

* `events`
* `pwm`
* `i2c`
* `spi`
* `uart`

### Module: `events`

* (class) `EventEmitter`
  * `addListener(eventName, listener)`
  * `emit(eventName, ...args)`
  * `on(eventName, listener)`
  * `once(eventName, listener)`
  * `removeAllListeners([eventName])`
  * `removeListener(eventName, listener)`

### Module: `pwm`

* (class) `PWM`
  * `setup(pin, freq, duty)`
    * `pin {number}`
    * `freq {number}` in Hz
    * `duty {number}` 0 ~ 1
  * `start()`
  * `stop()`
  * `getFrequency() -> number`
  * `setFrequency(freq)`
  * `getDuty() -> number`
  * `setDuty(duty)`
  * `close()`

  __Example:__
  ```js
  var PWM = require('pwm');
  var pwm = new PWM();
  pwm.setup(1, 1000, 0.5);
  pwm.start();
  // ...
  pwm.stop();
  pwm.close();
  ```

### Module: `i2c`

* (class) `I2C`
  * `setup(bus, ?address)`
    * `bus {number}` -- bus number 0, 1, 2...
    * `?address {number}` (optional) -- when use in slave mode. If given, open as slave mode
  * `write(data, ?timeout=0)` -- write data
    * `data {ArrayBuffer|Array<number>}`  -- Data to write
    * `?timeout=0 {number}`
  * `read(length, ?address, ?timeout=0) -> ArrayBuffer` -- read data as the length
    * `length {number}` -- data length
    * `address {number}` -- address to receive data (in master mode only)
    * `?timeout=0 {number}`
    * Return the received data
  * `close()` -- close the bus

  __Example:__
  ```js
  var I2C = require('i2c');
  var i2c = new I2C();
  i2c.setup(0); // open bus number 0
  i2c.write([0x66, 0x77]);
  var buf = i2c.read(10);
  i2c.close();
  ```

### Module: `spi`

* (class) `SPI`
  * (static) `MASTER` = 0
  * (static) `SLAVE` = 1
  * (static) `MSB` = 0
  * (static) `LSB` = 1
  * `setup(bus, mode, cs, ?baudrate=6000000, ?polarity=0, ?phase=0, ?bits=8, ?bitorder=MSB)`
    * `bus {number}`
    * `mode {number}` -- `SPI.MASTER` or `SPI.SLAVE`
    * `cs {number}` -- Chip select pin number
    * `?baudrate {number}` -- baudrate
    * `?polarity {number}` -- 0 or 1
    * `?phase {number}` -- 0 or 1
    * `?bits {number}` -- 8 or 9?
    * `?bitorder {number}` -- `SPI.MSB` or `SPI.LSB`
  * `transfer(data, ?timeout=5000)` -- Send and receive data simultaneously
    * `data {ArrayBuffer|Array<Number>|string}`
    * `?timeout=5000 {number}`
    * `@return {ArrayBuffer}` -- Received data
  * `send(data, ?timeout=5000)` -- Send data
    * `data {ArrayBuffer|Array<Number>|string}`
    * `?timeout=5000 {number}`
  * `recv(length, ?timeout=5000)` -- Receive data as the length
    * `length {number}`
    * `?timeout=5000 {number}`
    * `@return {ArrayBuffer}` -- Received data
  * `close()` -- close the bus

  __Example:__
  ```js
  var SPI = require('spi');
  var spi = new SPI();
  spi.setup(0, SPI.MASTER, 11 /*, ... */); // open bus number 0
  var received = spi.transfer([0x88, 0x24]); // Send two bytes and receive two bytes
  spi.send("data...");    // Send 7 bytes
  var buf = spi.recv(5);  // Receive 5 bytes
  spi.close();
  ```

### Module: `uart`

* (class) `UART`
  * `setup(bus, ?options)`
    * `bus` -- bus number
    * `?options` [Object] -- when use in slave mode. If given, open as slave mode
      * `.baudrate` [Number] -- default `9600`
      * `.bits` [Number] -- [5, 6, 7, 8, 9]? default `8`
      * `.parity` [Number] -- [-1 (none), 0, 1] default `-1`
      * `.stop`
      * `.flow`
      * `.timeout`
  * `write(data)` -- write data
    * `data {string|ArrayBuffer|Array<number>}` -- Data to write
  * `available()` -- returns available data length to read
  * `read(?length=1)` -- read bytes and send data to callback
    * `?length=1 {number}` -- data length
    * `@return {number|ArrayBuffer}`  -- Return a byte data when length is 1, or a buffer when length > 1.
  * `listen(callback)` -- start to listen receiving data and callback is called when data received. if callback is `null|undefined`, stop to listen.
    * `callback {function(ArrayBuffer)}`
  * `close()` -- close the bus

  __Example:__
  ```js
  var UART = require('uart');
  var uart = new UART();
  uart.open(0, {}); // open bus number 0
  uart.write("data..."); // send data
  var buf = uart.read(10); // read 10 bytes (blocking)
  uart.on('data', function (buf) {
    // called when data received.
  })
  uart.close();
  ```
