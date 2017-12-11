# API REFERENCE

## Globals

### Constants

* `HIGH` _(from gpio)_
* `LOW` _(from gpio)_
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

Advanced I/O

* `tone(pin, frequency, duration, duty?)`
* `noTone(pin)`
* `pulseIn()` -- ? (ref to Arduino)
* `pulseInLong()` -- ? (ref to Arduino)
* `shiftIn()` -- ? (ref to Arduino)
* `shiftOut()` -- ? (ref to Arduino)

Time

* `delay(msec)` _(from timers)_
* `millis()` _(from timers)_
* ~~`delayMicroseconds(microsec)`~~ (suspended)
* ~~`micros()`~~ (suspended)
* `setTimeout(fun, msec)` _(from timers)_
* `setInterval(fun, msec)` _(from timers)_
* `clearTimeout(timer)` _(from timers)_
* `clearInterval(timer)` _(from timers)_

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

* ...

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
