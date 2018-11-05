I2C
===

The `i2c` module supports communication with I2C (Inter-Integrated Circuit) / TWI (Two Wire Interface) deivces.

Use `require('i2c')` to access this module.

* [Class: I2C]()
  * [new I2C(bus[, address])]()
  * [write(data, address[, timeout])]()
  * [write(data[, timeout])]()
  * [read(length[, address[, timeout]])]()
  * [read(length[, timeout])]()
  * [close()]()

## Class: I2C

An instances of `I2C` represents a I2C bus.

### new I2C(bus[, address])

* __`bus`__ `{number}` Bus number.
* __`address`__ `{number}` Optional. Setup as slave mode with the given address. If not provided, setup as master mode.

This method setup an I2C bus or throws an exception if failed to setup. If the `address` parameter is given, setup as slave mode. Otherwise, setup as master mode.

```js
var I2C = require('i2c').I2C;

// open bus 0 in master mode
var i2c0 = new I2C(0); // equals to board.i2c(0)
// read or write ...
i2c0.close();

// open bus 1 in slave mode (address=0x7a)
var i2c1 = new I2C(1, 0x7a); // equals to board.i2c(1, 0x7a);
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

### write(data[, timeout]])

* __`data`__ `{ArrayBuffer|TypedArray|Array<number>|string}` Data to write
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{number}` The number of bytes written, `-1` if failed to write or timeout.

This method writes data to master device and returns the number of bytes written. This method can be called only in slave mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0, 0x7a); // slave mode

// Writes 2 bytes with an array of numbers
var array = [0x6b, 0x00];
i2c0.write(array);

// Writes 2 bytes with an instance of TypedArray or ArrayBuffer
var typedArray = new Uint8Array([0x6b, 0x00]);
i2c0.write(typedArray); // pass a typed array
i2c0.write(typedArray.buffer); // or, pass an array buffer

// Writes 2 bytes with a string
var str = 'abcdef';
i2c0.write(str);

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

### read(length[, timeout])

* __`length`__ `{number}` Data length to read.
* __`timeout`__ `{number}` Optional. Timeout in milliseconds. Default: `5000`.
* Returns: `{ArrayBuffer}` The data read.

This method read data from master device and returns an array buffer object. This method can be called only in slave mode.

```js
var I2C = require('i2c').I2C;
var i2c0 = new I2C(0, 0x7a); 
var buf = i2c0.read(10); // Read 10 bytes from the master.
i2c0.close();
var data = new Uint8Array(buf);
console.log(data.length); // 10
console.log(data[0]); // first byte
```

### close()

This method closes the bus.
