ADC (Analog to Digital Converter)
=================================

The `adc` module supports analog-digital conversion.

Use `require('adc')` to access this module.

* [Class: ADC]()
  * [new ADC(pin)]()
  * [read()]()
  * [pin]()

## Class: ADC

An instances of `ADC` represents a ADC object.

### new ADC(pin)

* __`pin`__ `{number}` Pin number.

```js
var ADC = require('adc');
var a3 = new ADC(3); // Pin @3.
var val = a3.read();

// or, create from board object
var a3 = board.adc(3); // Pin @3.
var val = a3.read();
```

### read()

* Returns: `{number}` Analog value of the pin.

This method returns the analog value read from the pin.

```js
var a3 = board.adc(3);
var value = a3.read();
```

### pin

* `{number}`

Pin number of the ADC object.

```js
var a3 = board.adc(3);
a3.pin; // 3
```
