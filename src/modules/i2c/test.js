var I2C = require('i2c'); var i2c = new I2C(); var address = 0x68; i2c.setup(0); i2c.write([0x6b, 0x00], address);
while (true) { i2c.write([0x3b], address); var d = new Uint8Array(i2c.read(6, address)); var x = (d[0] << 8) | d[1]; console.log(x); delay(1000); }
