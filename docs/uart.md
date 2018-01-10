UART
====

```js
var UART = require('uart');
```


## Class: UART


### setup(bus[, options])

* __`bus`__ `{number}` UART bus number.
* __`options`__ `{Object}` Options for the UART bus.
  * __`baudrate`__ `{number}` Baud rate. One of the `[0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400]`. Default is `9600`
  * __`bits`__ `{number}` Number of bits per a character. One of the `[7, 8, 9]`. Default is`8`
  * __`parity`__ `{number}`. The parity is one of the `UART.PARITY_NONE (=-1)`, `UART.PARITY_EVEN (=0)` or `UART.PARITY_ODD (=1)`. Default is `UART.PARITY_NONE`.
  * __`stop`__ `{number}` Number of stop bits. One of the `[1, 2]`. Default is `1`.
  * __`flow`__ `{number}` Flow control type. One of the `0`, `UART.RTS (=1)`, `UART.CTS (=2)`, or `UART.RTS | UART.CTS (=3)`. Default is `0`.
  * __`bufferSize`__ `{number}` The size of read buffer. Default is `1024`.
  * __`dataEvent`__ `{string|number}` If a character is given, 'data' event is emitted whenever the given character has arrived. If a number is given, 'data' event is emitted when buffer length is reached to the given number. For example: `'\n'`, `10`. Default is 0. This means try to trigger 'data' event whenever data is arrived.


```js
var UART = require('uart');
var uart = new UART();
uart.setup(0, {}); // open bus number 0
uart.write("data..."); // send data
var buf = uart.read(10); // read 10 bytes (blocking)
uart.on('data', function (buf) {
// called when data received.
})
uart.close();
```


### write(data)

Writes data to the UART bus.

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


### close()

Close the UART bus.


### Event: 'data'

Emitted when data has arrived.

* __`data`__ `{ArrayBuffer}`

```js
uart.on('data', function (data) {
console.log("Received data: " + data.toString());
});
```


### PARITY_NONE

* `{number}`


### PARITY_EVEN

* `{number}`


### PARITY_ODD

* `{number}`


### RTS

* `{number}`


### CTS

* `{number}`
