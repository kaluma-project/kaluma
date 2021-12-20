const uart_native = process.binding(process.binding.uart)
const {EventEmitter} = require('events');

function UART(port, options) {
  EventEmitter.call(this);
  let self = this;
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

UART.PARITY_NONE = uart_native.PARITY_NONE;
UART.PARITY_ODD = uart_native.PARITY_ODD;
UART.PARITY_EVEN = uart_native.PARITY_EVEN;

UART.FLOW_NONE = uart_native.FLOW_NONE;
UART.FLOW_RTS = uart_native.FLOW_RTS;
UART.FLOW_CTS = uart_native.FLOW_CTS;
UART.FLOW_RTS_CTS = uart_native.FLOW_RTS_CTS;

exports.UART = UART;
