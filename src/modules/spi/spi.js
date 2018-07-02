var spi_native = process.binding(process.binding.spi)

exports.open = function (bus, options) {
  if (options) {
    return new spi_native.SPI(bus, options);
  } else {
    return new spi_native.SPI(bus);
  }
}

exports.SPI = spi_native.SPI;
