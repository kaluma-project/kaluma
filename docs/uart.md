UART
====

The `uart` module supports communication with computers or deivces using serial ports (UART).

Use `require('uart')` to access this module.

* [open(bus[, options])]()
* [Class: UART]()
  * [new UART(port[, options])]()
  * [listen(callback[, trigger])]()
  * [close()]()
  * [Class Property: UART.PARITY_NONE]()
  * [Class Property: UART.PARITY_EVEN]()
  * [Class Property: UART.PARITY_ODD]()
  * [Class Property: UART.RTS]()
  * [Class Property: UART.CTS]()

## Class: UART

### setup(port[, options])

* __`port`__ `{number}` UART port number.
* __`options`__ `{Object}` Optional. Options for the UART port.
  * __`baudrate`__ `{number}` Optional. Baud rate. One of the `[0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400]`. Default is `9600`
  * __`bits`__ `{number}` Optional. Number of bits per a character. One of the `[7, 8, 9]`. Default is`8`
  * __`parity`__ `{number}`. Optional. The parity is one of the `UART.PARITY_NONE (=-1)`, `UART.PARITY_EVEN (=0)` or `UART.PARITY_ODD (=1)`. Default is `UART.PARITY_NONE`.
  * __`stop`__ `{number}` Optional. Number of stop bits. One of the `[1, 2]`. Default is `1`.
  * __`flow`__ `{number}` Optional. Flow control type. One of the `0`, `UART.RTS (=1)`, `UART.CTS (=2)`, or `UART.RTS | UART.CTS (=3)`. Default is `0`.
  * __`bufferSize`__ `{number}` Optional. The size of internal read buffer. Default is `1024`.


```js
var UART = require('uart');
var uart = new UART();
uart.setup(0, {}); // open port number 0
uart.write("data..."); // send data
var buf = uart.read(10); // read 10 bytes (blocking)
uart.on('data', function (buf) {
// called when data received.
})
uart.close();
```

### write(data)

Writes data to the UART port.

The write operation is non-blocking.

* __`data`__ `{string|ArrayBuffer|Array<number>}` -- Data to write


```js
// ...
```

### read([length])

Read data from read buffer as many as length. If no data in buffer, returns `null`.

* __`length`__ `{number}` Data length to read. Default is 1.
* Returns: `{null|ArrayBuffer}` -- Return data in the read buffer.


```js
// ...
```

### listen(callback[, trigger])

Callback function is called when data has arrived.

* __`callback`__ `{function(data)}` A function to be called whenever the internal read buffer is full or `trigger` condition has met.
  * __`data`__ `{ArrayBuffer}`
* __`trigger`__ `{number|string}` Optional. If a character is given, callback is called whenever the given character has arrived. If a number is given, callback is called whenever buffer length is reached to the given number. For example: `'\n'`, `10`. Default is `'\n'` (???). This means try to trigger 'data' event whenever data is arrived.

```js
uart0.listen(function (data) {
  console.log("Received number of bytes = " + data.byteLength);
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
