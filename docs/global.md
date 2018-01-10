Global
======

* [Object: global]()
  * [require(module_name)]()
  * [print([data][, ...args])]()
  * [LOW]()
  * [HIGH]()
  * [INPUT]()
  * [OUTPUT]()
  * [FALLING]()
  * [RISING]()
  * [CHANGE]()
  * [Digital I/O functions](digital_io.md)
  * [Analog I/O functions](analog_io.md)
  * [Timer functions](timers.md)
  * [Console object](console.md)
  * [Board object](board.md)
  * [Process object](process.md)

## Object: global

### require(module_name)

* __`module_name`__ `{string}`
* Returns: `{any}` Return `exports` object of the loaded module.

### print([data][, ...args])

Print data to console. It is equivalent to `console.log()`.

### LOW

* `{number}` = 0

### HIGH

* `{number}` = 1

### INPUT

* `{number}` = 0

### OUTPUT

* `{number}` = 1

### FALLING

* `{number}` = 0

### RISING

* `{number}` = 1

### CHANGE

* `{number}` = 2


## Constants

* `LOW` = 0
* `HIGH` = 1
* `INPUT` = 0
* `OUTPUT` = 1
* `FALLING` = 0
* `RISING` = 1
* `CHANGE` = 2

## Functions

[Digital I/O](digital_io.md)

* pinMode
* digitalRead
* digitalWrite
* digitalToggle
* setWatch
* clearWatch

[Analog I/O](analog_io.md)

* `analogRead(pin)`
* `analogWrite(pin, value)`
* `tone(pin, frequency, duration, ?duty)`
* `noTone(pin)`

[Timers](timers.md)

* `delay(msec)`
* `millis()`
* `setTimeout(callback, delay)`
* `setInterval(callback, delay)`
* `clearTimeout(id)`
* `clearInterval(id)`

Etc.

* `require(module)`
* `print(value)` -- equivalent to console.log

### Objects

* [global](global.md)
* `console`
* `process`
* `board`
