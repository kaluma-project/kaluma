UART
====

The `uart` module supports communication with computers or deivces using serial ports (UART).

Use `require('uart')` to access this module.

* [open(port[, options])]()
* [Class: UART]()
  * [new UART(port[, options])]()
  * [write(data)]()
  * [close()]()
  * [Event: 'data']()
  * [Class Property: UART.PARITY_NONE]()
  * [Class Property: UART.PARITY_ODD]()
  * [Class Property: UART.PARITY_EVEN]()
  * [Class Property: UART.RTS]()
  * [Class Property: UART.CTS]()

## open(port[, options])

* __`port`__ `{number}` UART port number.
* __`options`__ `{Object}` Optional. Same with the `options` parameter of [new UART(port[, options])]().
* Returns: `{UART}` Return an initalized instance of `UART`.

```js
var uart = require('uart');
var serial0 = uart.open(0);
// ...
serial0.close();
```

## Class: UART

An instances of `UART` represents a UART port.

### new UART(port[, options])

* __`port`__ `{number}` UART port number.
* __`options`__ `{Object}` Optional. Options for the UART port.
  * __`baudrate`__ `{number}` Optional. Baud rate. One of the `[0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400]`. Default is `9600`
  * __`bits`__ `{number}` Optional. Number of bits per a character. One of the `[7, 8, 9]`. Default is`8`
  * __`parity`__ `{number}`. Optional. The parity is one of the `UART.PARITY_NONE (=0)`, `UART.PARITY_ODD (=1)`, or `UART.PARITY_EVEN (=2)`. Default is `UART.PARITY_NONE`.
  * __`stop`__ `{number}` Optional. Number of stop bits. One of the `[1, 2]`. Default is `1`.
  * __`flow`__ `{number}` Optional. Flow control type. One of the `0`, `UART.RTS (=1)`, `UART.CTS (=2)`, or `UART.RTS | UART.CTS (=3)`. Default is `0`.
  * __`bufferSize`__ `{number}` Optional. The size of internal read buffer. Default is `1024`.
  * __`dataEvent`__ `{string|number}` Optional. A condition when the `data` event is emitted.


```js
var uart = require('uart');
var options = {
  baudrate: 9600,
  bits: 8,
  partity: uart.UART.PARTIY_NONE,
  stop: 1,
  flow: 0,
  bufferSize: 2048
};
var serial0 = new uart.UART(0, options);
// read or write data...
serial0.close();
```

### write(data)

* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to write.

Writes data to the UART port. The write operation is non-blocking.

```js
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });
serial0.write('Hello, world\n');
serial0.close();
```

### close()

Close the UART port.

### Event: 'data'

* __`data`__ `{ArrayBuffer}` Received data buffer.

The `data` event is emitted whenever data is arrived (buffer size may varies).

If the `dataEvent` option is given with a character (e.g. `'\n'`), this event is emitted whenever the given character has arrived. If the character is not arrived until the buffer is full, you will lose the data in the buffer.

If the `dataEvent` option is given with a number (e.g. `10`), this event is emitted whenever buffer length has reached to the given number. The number should be less then the buffer size.

```js
var uart = require('uart');

var options = {
  baudrate: 9600,
  dataEvent: '\n'
};

var serial0 = uart.open(0, options);

// The `data` event is emitted whenever '\n' is arrived.
serial0.on('data', function (data) {
  var line = String.fromCharCode.apply(null, new Uint8Array(data));
  console.log(line);
});
```


### Class Property: UART.PARITY_NONE

* `{number}` Value is `0`.


### Class Property: UART.PARITY_ODD

* `{number}` Value is `1`.


### Class Property: UART.PARITY_EVEN

* `{number}` Value is `2`.

### Class Property: UART.RTS

* `{number}` Value is `1`.


### Class Property: UART.CTS

* `{number}` Value is `2`.
