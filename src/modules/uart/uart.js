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

UART.PARITY_NONE = this._native.PARITY_NONE;
UART.PARITY_ODD = this._native.PARITY_ODD;
UART.PARITY_EVEN = this._native.PARITY_EVEN;

UART.FLOW_NONE = this._native.FLOW_NONE;
UART.FLOW_RTS = this._native.FLOW_RTS;
UART.FLOW_CTS = this._native.FLOW_CTS;
UART.FLOW_RTS_CTS = this._native.FLOW_RTS_CTS;

function open(bus, options) {
  return new UART(bus, options);
}

exports.open = open;
exports.UART = UART;
