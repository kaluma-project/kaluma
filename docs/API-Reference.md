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
  * (event) `onUncaughtException`
  * `arch`
  * `platform`
  * `version`
* `module` ?
* `exports` ?
* `__dirname` ?
* `__filename` ?
* `Serial`_n_
* `SPI`_n_
* `I2C`_n_

## Native Modules

* `assert`
* `buffer` - default in global
* `timers` - default in global
* `events`
* `net`
* `fs`
* `http`

