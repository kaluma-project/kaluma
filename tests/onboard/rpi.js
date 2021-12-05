const { Gpio } = require('onoff');
const { UART } = require('./uart')
const util = require('./util')

exports.tty = new UART('/dev/ttyACM0', { baudRate: 115200 })
exports.serial0 = new UART('/dev/serial0', { baudRate: 115200 })

exports.gpioRead = function (pin) {
  const gpio = new Gpio(pin, 'in');
  return gpio.readSync()
}

exports.gpioWrite = function (pin, val) {
  const gpio = new Gpio(pin, 'out');
  return gpio.writeSync(val)
}
