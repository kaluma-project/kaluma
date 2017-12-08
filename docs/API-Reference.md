# API REFERENCE

## Globals

### Constants

* `HIGH`
* `LOW`
* `INPUT`
* `OUTPUT`
* `INPUT_PULLUP`

### Functions

Digital I/O

* `digitalRead(pin)`
* `digitalWrite(pin, value)`
* `digitalToggle(pin)`
* `pinMode(pin, mode)`

Analog I/O

* `analogRead(pin)`
* `analogWrite(pin, value)`
* `analogReference()` -- ? (ref to Arduino)

Advanced I/O

* `tone(pin, frequency, duration, duty?)`
* `noTone(pin)`
* `pulseIn()` -- ? (ref to Arduino)
* `pulseInLong()` -- ? (ref to Arduino)
* `shiftIn()` -- ? (ref to Arduino)
* `shiftOut()` -- ? (ref to Arduino)

Time

* `delay(msec)`
* `delayMicroseconds(microsec)`
* `micros()`
* `millis()`
* `setTimeout(fun, msec)`
* `setInterval(fun, msec)`
* `clearTimeout(timer)`
* `clearInterval(timer)`

Etc.

* `require(module)`
* `print(value)`

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

* `console` (default, global)
* `buffer` (default, global)
* `timers` (default, global)
* `assert` (default)
* `events` (default)
* `i2c`
* `spi`
* `uart`
* `fs`
* `net`
* `http`

### Module: `console`

* (class) `Console`
  * `log()`
  * `warn()`
  * `error()`

### Module: `buffer`

* (class) `Buffer`

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
  * `eventNames()`
  * `getMaxListeners()`
  * `listenerCount(eventName)`
  * `listeners(eventName)`
  * `on(eventName, listener)`
  * `once(eventName, listener)`
  * `prependListener(eventName, listener)`
  * `prependOnceListener(eventName, listener)`
  * `removeAllListeners([eventName])`
  * `removeListener(eventName, listener)`
  * `setMaxListeners(n)`

### Module: `i2c`

* (class) `I2C`
  * `begin(address)`
  * `beginTransmission(address)`
  * `endTransmission()`
  * `write(data)`
  * `requestFrom()`
  * `available()` - Return the number of bytes available for read.
  * `read()` - Read a byte.

### Module: `spi`

* ...

### Module: `uart`

* ...

### Module: `fs`

* ...

### Module: `net`

* ...

### Module: `http`

* ...
