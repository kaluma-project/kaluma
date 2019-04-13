I2C
===

The `i2c` module supports communication with I2C (Inter-Integrated Circuit) / TWI (Two Wire Interface) deivces.

> * Slave Mode is NOT supported in this version but we'll suport it soon.

Use `require('i2c')` to access this module.

* [Class: I2C]()
  * [new I2C(bus[, mode[, mode_option]])]()
  * [write(data, address[, timeout])]()
  * [read(length, address[, timeout])]()
  * [memWrite(memAddress, data, address[, memAddr16bit[, timeout]])]()
  * [memRead(memAddress, length, address[, memAddr16bit[, timeout]])]()
  * [close()]()
  * [Class Property: I2C.MASTER]()
  * [Class Property: I2C.SLAVE]()
  * [Class Property: I2C.STDSPEED]()
  * [Class Property: I2C.FULLSPEED]()

## Class: I2C

An instances of `I2C` represents a I2C bus.

### new I2C(bus[, mode [, mode_option]])

* __`bus`__ `{number}` Bus number.
* __`mode`__ `{number}` Optional. Master or Slave mode (default is master mode)
* __`mode_option`__ `{number}` Clock speed for Master mode option and 7bit I2C slave address for Slave mode option.

This method setup an I2C bus or throws an exception if failed to setup. If the `address` parameter is given, setup as slave mode. Otherwise, setup as master mode.

```js
var I2C = require('i2c').I2C;

// open bus 0 in master mode
var i2c0 = new I2C(0); // equals to board.i2c(0)
// read or write ...
i2c0.close();

// open bus 1 in master mode, full speed
var i2c1 = new I2C(1, I2C.MASTER, I2C.FULLSPEED);
//  read or write ...
i2c1.close();
```

### write(data, address[, timeout])

* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to write.
* __`address`__ `{number}` Address to write data.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{number}` The number of bytes written, `-1` if failed to write or timeout.

This method writes data to the specified address (slave device) and returns the number of bytes written. This method can be called only in master mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0); // master mode

// Writes 2 bytes with an array of numbers
var array = [0x6b, 0x00];
i2c0.write(array, 0x68);

// Writes 2 bytes with an instance of TypedArray or ArrayBuffer
var typedArray = new Uint8Array([0x6b, 0x00]);
i2c0.write(typedArray, 0x68); // pass a typed array
i2c0.write(typedArray.buffer, 0x68); // or, pass an array buffer

// Writes 2 bytes with a string
var str = 'abcdef';
i2c0.write(str, 0x68);

i2c0.close();
```

### read(length, address[, timeout])

* __`length`__ `{number}` Data length to read.
* __`address`__ `{number}` Address to read data from (in master mode only).
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{ArrayBuffer}` An array buffer having data read, `null` if failed to read.

This method read data from the specified address (slave device) and returns an array buffer object. This method can be called only in master mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0); 
var buf = i2c0.read(14, 0x68); // Read 14 bytes from the address 0x68.
i2c0.close();
if (buf) {
  var data = new Uint8Array(buf);
  console.log(data.length); // 14
  console.log(data[0]); // first byte
}
```

### memWrite(memAddress, data, address[, memAddr16bit[, timeout]])

* __`memAddress`__ `{number}` Data length to read.
* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to write.
* __`address`__ `{number}` Address to write data.
* __`memAddr16bit`__ `{number}` set 1 when memAddress is 16bit address. set 0 when memAddress is 8bit address.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{number}` The number of bytes written, `-1` if failed to write or timeout.

This method writes data to the memory address in the specified slave device and returns the number of bytes written. This method can be called only in master mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0); // master mode

// Writes 2 bytes with an array of numbers
var array = [0x6b, 0x00];
i2c0.memWrite(0x10, array, 0x68);

// Writes 2 bytes with an instance of TypedArray or ArrayBuffer
var typedArray = new Uint8Array([0x6b, 0x00]);
i2c0.memWrite(0x10, typedArray, 0x68); // pass a typed array
i2c0.memWrite(0x10, typedArray.buffer, 0x68); // or, pass an array buffer

// Writes strings
var str = 'abcdef';
i2c0.memWrite(0x10, str, 0x68);

i2c0.close();
```

### memRead(memAddress, length, address[, memAddr16bit[, timeout]])

* __`memAddress`__ `{number}` Data length to read.
* __`length`__ `{number}` Data length to read.
* __`address`__ `{number}` I2C slave address to read data from.
* __`memAddr16bit`__ `{number}` set 1 when memAddress is 16bit address. set 0 when memAddress is 8bit address.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{ArrayBuffer}` An array buffer having data read, `null` if failed to read.

This method read data at memory address from the specified slave device and returns an array buffer object. This method can be called only in master mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0); 
var buf = i2c0.memRead(0x0100, 14, 0x68, 1); // Read 14 bytes at memory address 0x0100 from slave 0x68
i2c0.close();
if (buf) {
  var data = new Uint8Array(buf);
  console.log(data.length); // 14
  console.log(data[0]); // first byte
}
```

### close()

This method closes the bus.

### Class Property: I2C.MASTER

* `0`

### Class Property: I2C.SLAVE

* `1`

### Class Property: I2C.STDSPEED

* `100000`, 100 kbit/s I2C Standard speed

### Class Property: I2C.FULLSPEED

* `400000`, 400 kbit/s I2C full speed
