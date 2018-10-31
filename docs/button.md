Button
======

The `button` module supports button.

Use `require('button')` to access this module.

* [open(bus[, address])]()
* [Class: Button]()
  * [new Button(pin[, options])]()
  * [read()]()
  * [close()]()

## Class: Button

An instances of `Button` represents a button object.

### new Button(pin[, options])

* __`pin`__ `{number}` Pin number of the button.
* __`options`__ `{Object}` Optional.

```js
var Button = require('button');
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
