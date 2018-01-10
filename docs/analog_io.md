Analog I/O
==========

* [analogRead(pin)]()
* [analogWrite(pin, value[, frequency])]()
* [tone(pin, frequency[, duration[, duty]])]()
* [noTone(pin)]()

## analogRead(pin)

* __`pin`__ `{number}` Pin number.
* Returns: `{number}` Return value in between 0.0 ~ 1.0

Read value from an analog pin. Check the pinmap of the board for analog pins.

```js
var pin = board.adc(0); // Get an analog pin number
var value = analogRead(pin);
```


## analogWrite(pin, value[, frequency])

* __`pin`__ `{number}` Pin number.
* __`value`__ `{number}` Duty cycle between 0 and 1.
* __`frequency`__ `{number}` Frequency in Hz. Default is `490`Hz.(?)

Write value to an analog pin.

```js
var pin = board.adc(0); // Get an analog pin number
analogWrite(pin, 0.5);
```


## tone(pin, frequency[, duration[, duty]])

* __`pin`__ `{number}` Pin number.
* __`frequency`__ `{number}` Frequency in Hz. Default is `490`Hz.
* __`duration`__ `{number}` Duration in milliseconds. Default is 0.
* __`duty`__ `{number}` Duty cycle between 0 and 1.

Generate tone to the pin 0.

```js
tone(0, 300, 1000, 1);
```


## noTone(pin)

* __`pin`__ `{number}` Pin number.

Generate tone to the pin 0.

```js
tone(0, 200);
delay(1000);
noTone(0);
```
