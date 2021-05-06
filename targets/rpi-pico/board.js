global.board = {
  name: 'rpi-pico',
  LED: 25,
  gpio: function (...args) {
    var GPIO = global.require('gpio').GPIO;
    return new GPIO(...args);
  },
  led: function (...args) {
    var LED = global.require('led').LED;
    return new LED(...args);
  },
  button: function (...args) {
    var Button = global.require('button').Button;
    return new Button(...args);
  },
  pwm: function (...args) {
    var PWM = global.require('pwm').PWM;
    return new PWM(...args);
  },
  adc: function (...args) {
    var ADC = global.require('adc').ADC;
    return new ADC(...args);
  },
  i2c: function (...args) {
    var I2C = global.require('i2c').I2C;
    return new I2C(...args);
  },
  spi: function (...args) {
    var SPI = global.require('spi').SPI;
    return new SPI(...args);
  },
  uart: function (...args) {
    var UART = global.require('uart').UART;
    return new UART(...args);
  }
}
