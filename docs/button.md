Button
======

The `button` module supports button.

Use `require('button')` to access this module.

* [open(bus[, address])]()
* [Class: Button]()
  * [new Button(pin[, pull[, debounce]])]()
  * [read()]()
  * [close()]()
  * [Event: 'click']()

## Class: Button

An instances of `Button` represents a button object.

This class is a subclass of `EventEmitter`.

### new Button(pin[, pull[, debounce]])

* __`pin`__ `{number}` Pin number of the button.
* __`pull`__ `{number}` Optional. `PULL_UP` or `PULL_DOWN`. Default is `PULL_DOWN`.
* __`debounce`__ `{number}` Optional. Default is `50`.

```js
var Button = require('button').Button;
var btn0 = new Button(board.button_pins[0]);
btn0.on('click', function () {
  console.log('button 1 clicked');
})
// ...
btn0.close();
```

### read()

* Returns: `{number}`

### close()

This method closes the I/O watcher for the button.

### Event: 'click'

The `click` event is emitted when the button is pressed down.

```js
var btn = board.BTN0;
btn.on('click', function () {
  board.LED0.toggle();
})
```
