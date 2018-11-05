LED
===

The `led` module supports LED.

Use `require('led')` to access this module.

* [Class: LED]()
  * [new LED(pin)]()
  * [on()]()
  * [off()]()
  * [toggle()]()
  * [read()]()
  * [pin]()

## Class: LED

An instances of `LED` represents a LED.

### new LED(pin)

* __`pin`__ `{number}` Pin number where LED connected.

```js
var LED = require('led').LED;
var led = new LED(20); // LED connected to pin 20.

// For on-board LED(s)
var led = board.led(0); // On-board LED0
```

### on()

This method turns on the LED.

```js
var led = board.led(0);
led.on();
```

### off()

This method turns off the LED.

```js
var led = board.led(0);
led.off();
```

### toggle()

This method toggles the LED.

```js
var led = board.led(0);
pin.off();
pin.toggle(); // on
pin.toggle(); // off
```

### read()

* Returns: `{number}` State of the LED.

This method returns the state of the LED.

```js
var led = board.led(0);
led.on();
led.read(); // Returns 1.
led.off();
led.read(); // Returns 0.
```

### pin

* `{number}`

Pin number of the LED.
