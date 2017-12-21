# API REFERENCE

## Globals

### Constants

* `HIGH` = 1 _(from gpio)_
* `LOW` = 0 _(from gpio)_
* `INPUT` _(from gpio)_
* `OUTPUT` _(from gpio)_
* ...

### Functions

Digital I/O

* `digitalRead(pin)` _(from gpio)_
* `digitalWrite(pin, value)` _(from gpio)_
* `digitalToggle(pin)` _(from gpio)_
* `pinMode(pin, mode)` _(from gpio)_

Analog I/O

* `analogRead(pin)`
* `analogWrite(pin, value)`
* `analogReference()` -- ? (ref to Arduino)

~~Advanced I/O~~

* ~~`tone(pin, frequency, duration, ?duty)`~~ (suspended)
* ~~`noTone(pin)` ~~ (suspended)
* ~~`pulseIn()`~~ (suspended)
* ~~`pulseInLong()`~~ (suspended)
* ~~`shiftIn()`~~ (suspended)
* ~~`shiftOut()`~~ (suspended)

Non-blocking I/O

* `FALLING` = 0
* `RISING` = 1
* `CHANGE` = 2
* `setWatch(callback, pin, ?mode=CHANGE, ?debounce=0) -> id` _(from gpio)_
  * `callback {Function: () -> undefined}`
  * `pin {number}`
  * `?mode=CHANGE {number}` -- RISING | FALLING | CHANGE
  * `?debounce=0 {number}`
* `clearWatch(id)` _(from gpio)_

Time

* `delay(msec)` _(from timers)_
* `millis()` _(from timers)_
* ~~`delayMicroseconds(microsec)`~~ (suspended)
* ~~`micros()`~~ (suspended)
* `setTimeout(callback, msec) -> id` _(from timers)_
* `setInterval(callback, msec) -> id` _(from timers)_
* `clearTimeout(id)` _(from timers)_
* `clearInterval(id)` _(from timers)_

Etc.

* `require(module)`
* `print(value)` - equivalent to console.log

### Objects

* (class) `Buffer`
* `global`
* `console`
  * `log()`
  * `warn()`
  * `error()`
* `process`
  * `builtin_modules` - _array of builtin module names_
  * `binding(native_module_name)` - _load native modules_
    * `<native_module_name>` - _binding has native module names as properties_
  * `arch`
  * `platform`
  * `version`
  * (event) `onUncaughtException`
* `module` ?
* `exports` ?
* `__dirname` ?
* `__filename` ?
* `Serial`_n_
* `SPI`_n_
* `I2C`_n_

## Builtin Modules

* `console` (global)
* `buffer` (global)
* `timers` (global)
* `assert`
* `events`
* `gpio` (global)
* `pwm`
* `i2c`
* `spi`
* `uart`
* `fs`
* `net`
* `http`

### Module: `console`

* `log(...)`
* ~~`info(...)`~~ (suspended)
* ~~`warn(...)`~~ (suspended)
* `error(...)`

### Module: `buffer`

* (class) `Buffer` _(=exports)_

### Module: `timers`

* `setInterval(callback, interval)`
* `setTimeout(callback, timeout)`
* `clearInterval(timerId)`
* `clearTimeout(timerId)`

### Module: `assert`

* ...

### Module: `events`

* (class) `EventEmitter`
  * `addListener(eventName, listener)`
  * `emit(eventName, ...args)`
  * `on(eventName, listener)`
  * `once(eventName, listener)`
  * `removeAllListeners([eventName])`
  * `removeListener(eventName, listener)`
  * ~~`eventNames()`~~ (suspended)
  * ~~`getMaxListeners()`~~ (suspended)
  * ~~`listenerCount(eventName)`~~ (suspended)
  * ~~`listeners(eventName)`~~ (suspended)
  * ~~`prependListener(eventName, listener)`~~ (suspended)
  * ~~`prependOnceListener(eventName, listener)`~~ (suspended)
  * ~~`setMaxListeners(n)`~~ (suspended)

### Module: `gpio`

* (const) `HIGH`
* (const) `LOW`
* (const) `INPUT`
* (const) `OUTPUT`
* `digitalRead(pin)`
* `digitalWrite(pin, value)`
* `digitalToggle(pin)`
* `pinMode(pin, mode)`

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

### Module: `adc`

* (class) `ADC`
  * `setup(pin)`
    * `pin {number}`
  * `read() -> number`
  * `close()`

  __Example:__
  ```js
  var ADC = require('adc');
  var adc = new ADC();
  adc.setup(1);
  var val = adc.read();
  adc.close();
  ```

### Module: `i2c`

* (class) `I2C`
  * `open(bus, ?address)`
    * `bus {number}` -- bus number 0, 1, 2...
    * `?address {number}` (optional) -- when use in slave mode. If given, open as slave mode
  * `write(data, ?timeout=0)` -- write data
    * `data {Buffer|Array<number>}`  -- Data to write
    * `timeout {number}`
  * `read(data, length)` -- read bytes and send data to callback
    * `data {Buffer|Array<number>}` -- Read and store in data
    * `length {number}` -- data length
  * `close()` -- close the bus

  __Example:__
  ```js
  var I2C = require('i2c');
  var i2c = new I2C();
  i2c.open(0); // open bus number 0
  i2c.write([0x66, 0x77]);
  i2c.read(?); // how to do it?
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
  * `send(data, ?timeout=5000)`
    * `data {Buffer|Array<Number>|string}`
    * `?timeout=5000 {number}`
  * `recv(size, ?timeout=5000) -> Buffer`
    * `size {number}`
    * `?timeout=5000 {number}`
  * `close()` -- close the bus

  __Example:__
  ```js
  var SPI = require('spi');
  var spi = new SPI();
  spi.setup(0, SPI.MASTER, 11 /*, ... */); // open bus number 0
  spi.send("data...");
  // ...
  var buf = spi.recv(5);
  // ...
  spi.close();
  ```

### Module: `uart`

* (class) `UART`
  * `open(bus, ?options)`
    * `bus` -- bus number
    * `?options` [Object] -- when use in slave mode. If given, open as slave mode
      * `.baudrate` [Number] -- default `9600`
      * `.bits` [Number] -- [5, 6, 7, 8, 9]? default `8`
      * `.parity` [Number] -- [-1 (none), 0, 1] default `-1`
      * `.stop`
      * `.flow`
      * `.timeout`
  * `write(data)` -- write data
    * `data` [String|Buffer|Number|Array of Number] -- Data to write
  * `read(data, length)` -- read bytes and send data to callback
    * `data` [Buffer|Array of Number] -- Read and store in data
    * `length` -- data length
  * `close()` -- close the bus

  __Example:__
  ```js
  var UART = require('uart');
  var uart = new UART();
  uart.open(0, {}); // open bus number 0
  uart.write("data...");
  // ...
  uart.close();
  ```

### Module: `fs`

* ...

### Module: `net`

* ...

### Module: `http`

* ...
