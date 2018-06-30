var i2c = require('i2c'); var bus0 = new i2c.open(0); var address = 0x68; var buf = new Uint8Array([0x6b, 0x00]); bus0.write(buf, address);
while (true) { bus0.write([0x3b], address); var d = new Uint8Array(bus0.read(6, address)); var x = (d[0] << 8) | d[1]; console.log(x); delay(1000); }
