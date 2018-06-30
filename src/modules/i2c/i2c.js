var i2c_native = process.binding(process.binding.i2c)

exports.open = function (bus, address) {
  if (address) {
    return new i2c_native.I2C(bus, address);
  } else {
    return new i2c_native.I2C(bus);
  }
}

exports.I2C = i2c_native.I2C;
