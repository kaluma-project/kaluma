// uart write
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });
serial0.write('Hello, world\n');

// uart read
var uart = require('uart');
var serial0 = uart.open(0, { baudrate: 9600 });

while (true) { if (serial0.availabe()) { var buf = serial0.read(); console.log(buf[0]); } }

console.log(String.fromCharCode.apply(null, new Uint8Array(serial0.read(serial0.available()))));

// uart event 'data'

var uart = require('uart'); var serial0 = uart.open(0, { baudrate: 9600, bufferSize: 100, dataEvent: '@' }); serial0.on('data', function (data) { var line = String.fromCharCode.apply(null, new Uint8Array(data)); console.log(line); });

serial0.removeAllListeners('data');
serial0.close();
