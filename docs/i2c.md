I2C
===

* [Class: I2C]()
  * [setup(bus[, address])]()
  * [write(data[, timeout])]()
  * [read(length[, address[, timeout]])]()
  * [close()]()


## Class: I2C

```js
var I2C = require('i2c');
var i2c = new I2C();
i2c.setup(0); // open bus number 0
i2c.write([0x66, 0x77]);
var buf = i2c.read(10);
i2c.close();
```

### setup(bus[, address])

* __`bus`__ `{number}` bus number 0, 1, 2...
* __`address`__ `{number}` Optional. -- when use in slave mode. If given, open as slave mode

### write(data[, timeout])

* __`data`__ `{ArrayBuffer|Array<number>}` Data to write
* __`timeout`__ `{number}`

### read(length[, address[, timeout]])

* __`length`__ `{number}` data length to read.
* __`address`__ `{number}` Optional. address to receive data (in master mode only).
* __`timeout`__ `{number}` Optional.
* Returns: `{ArrayBuffer}` the received data

### close()

close the bus

