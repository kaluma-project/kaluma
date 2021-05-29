var spi = require('spi');
var SS = 19;
pinMode(SS, OUTPUT);
digitalWrite(SS, HIGH);
var spi0 = spi.open(0, { mode: 0, baudrate: 9600 });
var str = "hello, world\n";
digitalWrite(SS, LOW); spi0.transfer(str); digitalWrite(SS, HIGH);
