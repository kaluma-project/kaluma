I2C
===

* [Class: I2C]()
  * [setup(bus[, address])]()
  * [write(data, address[, timeout])]()
  * [write(data[, timeout])]()
  * [read(length[, address[, timeout]])]()
  * [read(length[, timeout])]()
  * [close()]()


## Class: I2C

```js
var I2C = require('i2c');
var i2c = new I2C();
i2c.setup(0); // open bus number 0 in master mode
i2c.write([0x66, 0x77], 0x0c, 3000); // send data to the address 0x0c with timeout 3 seconds.
var buf = i2c.read(10, 0x0c); // read 10 bytes from the address 0x0c
i2c.close();
```

```js
var I2C = require('i2c');
var i2c = new I2C();
i2c.setup(0, 0x7a); // open bus number 0 in slave mode (address 0x7a)
i2c.write([0x66, 0x77], 3000);
var buf = i2c.read(10);
i2c.close();
```

### setup(bus[, address])

* __`bus`__ `{number}` bus number 0, 1, 2...
* __`address`__ `{number}` Optional. If given, open as slave mode. Otherwise, open as master mode.

### write(data, address[, timeout])

* __`data`__ `{string|ArrayBuffer|Array<number>}` Data to write
* __`address`__ `{number}` Address of slave device
* __`timeout`__ `{number}` Optional. Default is 5000.

Write data to the specified address (slave device). This method can be called only in master mode.

### write(data[, timeout]])

* __`data`__ `{string|ArrayBuffer|Array<number>}` Data to write
* __`timeout`__ `{number}` Optional. Default is 5000.

Write data to master device. This method can be called only in slave mode.

### read(length, address[, timeout])

* __`length`__ `{number}` data length to read.
* __`address`__ `{number}` Optional. address to receive data (in master mode only).
* __`timeout`__ `{number}` Optional. Default is 5000.
* Returns: `{ArrayBuffer}` the received data

Read data from the specified address (slave device). This method can be called only in master mode.

### read(length[, timeout])

* __`length`__ `{number}` data length to read.
* __`timeout`__ `{number}` Optional. Default is 5000.
* Returns: `{ArrayBuffer}` the received data

Read data from master device. This method can be called only in slave mode.

### close()

Close the bus.
