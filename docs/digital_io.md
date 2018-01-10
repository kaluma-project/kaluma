Digital I/O
===========

* [pinMode(pin[, mode])]()
* [digitalRead(pin)]()
* [digitalWrite(pin[, value])]()
* [digitalToggle(pin)]()
* [setWatch(callback, pin[, mode[, debounce]])]()
* [clearWatch(id)]()

## pinMode(pin[, mode])

* __`pin`__ `{number}` Pin number.
* __`mode`__ `{number}` Pin mode `INPUT` or `OUTPUT`.

Set pin 1 to HIGH.

```js
pinMode(1, OUTPUT);
digitalWrite(1, HIGH);
```


## digitalRead(pin)

* __`pin`__ `{number}` Pin number.
* Returns: `{number}` Read value from the pin.

Read value from pin 1.

```js
pinMode(1, INPUT);
var value = digitalRead(1);
```


## digitalWrite(pin[, value])

* __`pin`__ `{number}`
* __`value`__ `{number}` : default is 0.

```js
pinMode(1, OUTPUT);
digitalWrite(1, HIGH);
```


## digitalToggle(pin)

* __`pin`__ `{number}`

```js
digitalToggle(pin)
```


## setWatch(callback, pin[, mode[, debounce]])

* __`callback`__ `{function()}`
* __`pin`__ `{number}`
* __`mode`__ `{number}` -- RISING | FALLING | CHANGE
* __`debounce`__ `{number}`
* Returns: `{number}` Watcher id.

Whenever press the first button in the board, prints `click` string to terminal.

```js
var button = board.switch(0); // board's first button
var id = setWatch(function () {
console.log("click");
}, button, RISING, 10);
```


## clearWatch(id)

* __`id`__ `{number}` Watcher id.

Remove the watcher of the id.

```js
var id = setWatch(...);
clearWatch(id);
```
