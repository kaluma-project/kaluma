UART
====

The `uart` module supports communication with computers or deivces using serial ports (UART).

Use `require('uart')` to access this module.

* [open(port[, options])]()
* [Class: UART]()
  * [new UART(port[, options])]()
  * write(data)
  * read([length])
  * [listen(callback[, trigger])]()
  * [close()]()
  * [Class Property: UART.PARITY_NONE]()
  * [Class Property: UART.PARITY_EVEN]()
  * [Class Property: UART.PARITY_ODD]()
  * [Class Property: UART.RTS]()
  * [Class Property: UART.CTS]()

## open(port[, options])

* __`port`__ `{number}` UART port number.
* __`options`__ `{Object}` Optional. Same with the `options` parameter of [new UART(port[, options])]().
* Returns: `{UART}` Return an initalized instance of `UART`.

```js
var uart = require('uart');
var serial0 = uart.open(0);
// read or write data...
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

### available()

* Returns: `{number}` Returns the length of data in the read buffer.

Returns the length of data in read buffer.

```js
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });
while (true) {
  if (serial0.availabe()) {
    var buf = serial0.read();
    console.log(buf[0]);
  }
}
```

### read([length])

Read data from read buffer as many as length. If no data in buffer, returns `null`.

* __`length`__ `{number}` Data length to read. Default is 1.
* Returns: `{null|ArrayBuffer}` -- Return data in the read buffer.

```js
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });

// Read all data in the read buffer
var size = serial0.available();
var buf = serial0.read(size);
```

### listen(callback[, trigger])

* __`callback`__ `{function(data)}` A function to be called whenever the internal read buffer is full or `trigger` condition has met.
  * __`data`__ `{ArrayBuffer}`
* __`trigger`__ `{number|string}` Optional. If a character is given, callback is called whenever the given character has arrived. If a number is given, callback is called whenever buffer length is reached to the given number. For example: `'\n'`, `10`. Default is `'\n'` (???). This means try to trigger 'data' event whenever data is arrived.

Callback function is called when data has arrived.

```js
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });

// Callback is called whenever newline char ('\n') arrived.
serial0.listen(function (data) {
  var line = String.fromCharCode.apply(null, new Uint8Array(data));
  console.log(line);
}, '\n');
```

### close()

Close the UART port.

### port

* `{number}`

### baudrate

* `{number}`

### bits

* `{number}`

### parity

* `{number}`

### stop

* `{number}`

### flow

* `{number}`

### bufferSize

* `{number}`

### Class Property: UART.PARITY_NONE

* `{number}`


### Class Property: UART.PARITY_EVEN

* `{number}`


### Class Property: UART.PARITY_ODD

* `{number}`


### Class Property: UART.RTS

* `{number}`


### Class Property: UART.CTS

* `{number}`
