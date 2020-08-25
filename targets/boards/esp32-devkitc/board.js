global.board = {
  name: 'esp32-devkitc',
  NUM_GPIO: 33,
  NUM_LED: 0,
  NUM_BUTTON: 1,
  NUM_PWM: 0,
  NUM_ADC: 0,
  NUM_I2C: 0,
  NUM_SPI: 0,
  NUM_UART: 0,
  led_pins: [],
  button_pins: [],
  pwm_pins: [],
  adc_pins: [],
  gpio: function (pin, mode) {
    var GPIO = global.require('gpio').GPIO;
    return new GPIO(pin, mode);
  },
  led: function (pin) {
    var LED = global.require('led').LED;
    return new LED(pin);
  },
  button: function (pin, pull, debounce) {
    var Button = global.require('button').Button;
    return new Button(pin, pull, debounce);
  },
  pwm: function (pin, frequency, duty) {
    if (this.pwm_pins.indexOf(pin) < 0) {
      throw Error('The pin is not PWM capable.');
    }
    var PWM = global.require('pwm').PWM;
    frequency = (typeof frequency === 'number' ? frequency : 490);
    duty = (typeof duty === 'number' ? duty : 1);
    return new PWM(pin, frequency, duty);
  },
  adc: function (pin) {
    if (this.adc_pins.indexOf(pin) < 0) {
      throw Error('The pin is not ADC capable.');
    }
    var ADC = global.require('adc').ADC;
    return new ADC(pin);
  },
  i2c: function (bus, mode, mode_option) {
    if (bus < 0 || bus >= this.NUM_I2C) {
      throw Error('Unsupported I2C bus.');
    }
    var I2C = global.require('i2c').I2C;
    mode = (typeof mode === 'number' ? mode : 0);
    mode_option = (typeof mode_option === 'number' ? mode_option : 100000);
    return new I2C(bus, mode, mode_option);
  },
  spi: function (bus, options) {
    if (bus < 0 || bus >= this.NUM_SPI) {
      throw Error('Unsupported SPI bus.');
    }
    var SPI = global.require('spi').SPI;
    if (arguments.length > 1) {
      return new SPI(bus, options);
    } else {
      return new SPI(bus);
    }
  },
  uart: function (port, options) {
    if (port < 0 || port >= this.NUM_UART) {
      throw Error('Unsupported UART port.');
    }
    var UART = global.require('uart').UART;
    if (arguments.length > 1) {
      return new UART(port, options);
    } else {
      return new UART(port);
    }
  },
  get BTN0() {
    if (!this._btn0) {
      var Button = global.require('button').Button;
      this._btn0 = new Button(this.button_pins[0], FALLING);
    }
    return this._btn0;
  }
}
