global.board = {
  name: 'rpi-pico',
  LED: 25,
  gpio: function (pin, mode) {
    var GPIO = global.require('gpio').GPIO;
    return new GPIO(pin, mode);
  },
  led: function (pin) {
    var LED = global.require('led').LED;
    return new LED(pin);
  },
  button: function (pin, options) {
    var Button = global.require('button').Button;
    return new Button(pin, options);
  },
  pwm: function (pin, frequency, duty) {
    var PWM = global.require('pwm').PWM;
    return new PWM(pin, frequency, duty);
  },
  adc: function (pin) {
    var ADC = global.require('adc').ADC;
    return new ADC(pin);
  },
  i2c: function (bus, options) {
    var I2C = global.require('i2c').I2C;
    return new I2C(bus, options);
  },
  spi: function (bus, options) {
    var SPI = global.require('spi').SPI;
    return new SPI(bus, options);
  },
  uart: function (port, options) {
    var UART = global.require('uart').UART;
    return new UART(port, options);
  }
}
