var uart_native = process.binding(process.binding.uart)
var EventEmitter = require('events').EventEmitter;

function UART(port, options) {
  EventEmitter.call(this);
  var self = this;
  options = options || {};
  this._native = new uart_native.UART(port, options, function (data) {
    self.emit('data', data);
  });
}

// Inherits from EventEmitter
UART.prototype = new EventEmitter();
UART.prototype.constructor = UART;

UART.prototype.write = function (data) {
  this._native.write(data);
}

UART.prototype.close = function () {
  this._native.close();
}

UART.PARITY_NONE = 0;
UART.PARITY_ODD = 1;
UART.PARITY_EVEN = 2;

UART.RTS = 1;
UART.CTS = 2;

function open(bus, options) {
  return new UART(bus, options);
}

exports.open = open;
exports.UART = UART;
