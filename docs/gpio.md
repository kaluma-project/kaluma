GPIO (General-Purpose Input/Output)
===================================

The `gpio` module supports GPIO pin abstraction.

Use `require('gpio')` to access this module.

* [Class: GPIO]()
  * [new GPIO(pin[, mode])]()
  * [read()]()
  * [write(value)]()
  * [toggle()]()
  * [setMode(mode)]()
  * [pin]()
  * [mode]()

## Class: GPIO

An instances of `GPIO` represents a GPIO pin.

### new GPIO(pin[, mode])

* __`pin`__ `{number}` Pin number.
* __`mode`__ `{number}` Optional. Pin mode.

```js
var GPIO = require('gpio').GPIO;
var pin = new GPIO(0, OUTPUT);

// or, shortly
var pin = board.gpio(0, OUTPUT);
```

### read()

* Returns: `{number}` Value of the pin.

This method returns the value read from the GPIO pin.

```js
var pin = board.gpio(0, INPUT);
var value = pin.read();
```

### write(value)

* __`value`__ `{number}` Value to write.

This method writes a value to the GPIO pin.

```js
var pin = board.gpio(0, OUTPUT);
pin.write(1);
```

### toggle()

This method toggles the value of the GPIO pin.

```js
var pin = board.gpio(0, OUTPUT);
pin.write(0); // Set to LOW
pin.toggle(); // HIGH
pin.toggle(); // LOW
```

### pin

* `{number}`

Pin number of the GPIO object.

### mode

* `{number}`

Current mode of the GPIO pin.
