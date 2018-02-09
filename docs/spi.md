SPI
===

* [Class: SPI]()
  * [setup(bus[, options])]()
  * [transfer(data[, timeout])]()
  * [send(data[, timeout])]()
  * [recv(length[, timeout])]()
  * [close()]()
  * [MASTER]()
  * [SLAVE]()
  * [MSB]()
  * [LSB]()


## Class: SPI

__Example:__
```js
var SPI = require('spi');
var spi = new SPI();
spi.setup(0, SPI.MASTER, 11 /*, ... */); // open bus number 0
var received = spi.transfer([0x88, 0x24]); // Send two bytes and receive two bytes
spi.send("data..."); // Send 7 bytes
var buf = spi.recv(5); // Receive 5 bytes
spi.close();
```

### setup(bus[, options])

* __`bus`__ `{number}`
* __`options`__ `{Object}`
  * `mode` `{number}` -- `SPI.MASTER` (default) or `SPI.SLAVE`
  * `cs` `{number}` -- Chip select pin number
  * `clock` `{number}` -- clock
  * `polarity` `{number}` -- 0 or 1
  * `phase` `{number}` -- 0 or 1
  * `bits` `{number}` -- 8 or 9?
  * `bitorder` `{number}` -- `SPI.MSB` or `SPI.LSB`

### transfer(data[, timeout])

* __`data`__ `{ArrayBuffer|Array<number>|string}`
* __`timeout`__ `{number}`
* Returns: `{ArrayBuffer}` -- Received data

Send and receive data simultaneously

### send(data[, timeout])

* __`data`__ `{ArrayBuffer|Array<Number>|string}`
* __`timeout`__ `{number}`

Send data

### recv(length[, timeout])

* __`length`__ `{number}`
* __`timeout`__ `{number}`
* Returns: `{ArrayBuffer}` -- Received data

Receive data as the length

### close()

close the bus


### MASTER

* `{number}`

### SLAVE

* `{number}`

### MSB

* `{number}`

### LSB

* `{number}`
