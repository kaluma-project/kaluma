var uart_native = process.binding(process.binding.uart)

exports.open = function (bus, options) {
  if (options) {
    return new uart_native.UART(bus, options);
  } else {
    return new uart_native.UART(bus);
  }
}

exports.UART = uart_native.UART;
