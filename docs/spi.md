SPI
===

The `spi` module supports communication with deivces using SPI (Serial Peripheral Interface) bus as the master device.

Use `require('spi')` to access this module.

---
## 질문:
- Chip Select 핀 번호는 별도로 사용자가 컨트롤 해주어야 할듯 하다. 맞나?
- `mode` 디폴트값은 무엇을 줄것인가? SPI.MODE_0?
- `baudrate` 디폴트값은 무엇을 줄것인가? 사용자는 값을 어떻게 주어야 하는가?
- `bitsorder` 디폴트값은 무엇을 줄것인가? MSB? or LSB?
- `bits` 디폴트값은 무엇을 줄것인가? 8? or 16? 둘 중 하나만 가능한가?
---

* [open(bus[, options])]()
* [Class: SPI]()
  * [new SPI(bus[, options])]()
  * [transfer(data[, timeout])]()
  * [send(data[, timeout])]()
  * [recv(length[, timeout])]()
  * [close()]()
  * [Class Property: SPI.MODE_0]()
  * [Class Property: SPI.MODE_1]()
  * [Class Property: SPI.MODE_2]()
  * [Class Property: SPI.MODE_3]()
  * [Class Property: SPI.MSB]()
  * [Class Property: SPI.LSB]()

## open(bus[, options])

* __`bus`__ `{number}` Bus number.
* __`options`__ `{Object}` Optional. Same with the `options` parameter of [new SPI(bus[, options])]().
* Returns: `{SPI}` Return an initalized instance of `SPI`.

```js
var spi = require('spi');
var spi0 = spi.open(0);
// transfer data...
spi0.close();
```

## Class: SPI

An instances of `SPI` represents a SPI bus.

### new SPI(bus[, options])

* __`bus`__ `{number}` Bus number.
* __`options`__ `{Object}` Optional.
  * `mode` `{number}` Optional. `SPI.MODE_0` (CPOL=0/CPHA=0), `SPI.MODE_1` (CPOL=0/CPHA=1), `SPI.MODE_2` (CPOL=1/CPHA=0), or `SPI.MODE_3` (CPOL=1/CPHA=1). Please see [here](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase) for more about SPI modes. Default: `SPI.MODE_0`.
  * `baudrate` `{number}` Optional. Baud rate. Default: ?
  * `bitorder` `{number}` Optional. `SPI.MSB` or `SPI.LSB`
  * `bits` `{number}` Optional. `8` or `16`.

```js
var spi = require('spi');
var options = {
  mode: spi.SPI.MODE_0,
  baudrate: 14000000,
  bitoder: spi.SPI.MSB,
  bits: 8
};
var spi0 = new spi.SPI(0, options); // equivalent to spi.open(0, options);
// transfer data...
spi0.close();
```

### transfer(data[, timeout])

* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to transfer.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{ArrayBuffer}` Received data or `null` if failed to transfer or timeout.

Send and receive data simultaneously.

```js
var spi = require('spi');
var spi0 = spi.open(0);

// Send two bytes and receive two bytes
var buf = spi0.transfer([0x88, 0x24]);
if (buf) {
  var data = new Uint8Array(buf);
  console.log(data.length); // == 2
  console.log(data[0]); // first byte
  console.log(data[1]); // second byte
}

spi0.close();
```

### send(data[, timeout])

* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to send.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{number}` The number of bytes written, `-1` if failed to send or timeout.

Send data

```js
var spi = require('spi');
var spi0 = spi.open(0);

// Send 2 bytes with an array of numbers
var array = [0x6b, 0x00];
spi0.send(array);

// Send 2 bytes with an instance of TypedArray or ArrayBuffer
var typedArray = new Uint8Array([0x6b, 0x00]);
spi0.send(typedArray); // pass a typed array
spi0.send(typedArray.buffer); // or, pass an array buffer

// Send 2 bytes with a string
var str = 'abcdef';
spi0.send(str);

spi0.close();
```

### recv(length[, timeout])

* __`length`__ `{number}` Data length to receive.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{ArrayBuffer}` Received data or `null` if failed to receive or timeout.

Receive data as the length.

```js
var spi = require('spi');
var spi0 = spi.open(0);

// Receive 10 bytes
var buf = spi0.recv(10);
if (buf) {
  var data = new Uint8Array(buf);
  console.log(data.length); // == 10
  console.log(data[0]); // first byte
  console.log(data[1]); // second byte
  // ...
}

spi0.close();
```

### close()

This method closes the bus.


### Class Property: SPI.MODE_0

* `{number}`

### Class Property: SPI.MODE_1

* `{number}`

### Class Property: SPI.MODE_2

* `{number}`

### Class Property: SPI.MODE_3

* `{number}`

### Class Property: SPI.MSB

* `{number}`

### Class Property: SPI.LSB

* `{number}`
