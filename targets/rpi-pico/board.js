global.board = {
  name: 'rpi-pico',
  NUM_GPIO: 26,
  NUM_LED: 1,
  NUM_BUTTON: 0,
  NUM_PWM: 16,
  NUM_ADC: 3,
  NUM_I2C: 2,
  NUM_SPI: 2,
  NUM_UART: 2,
  led_pins: [25],
  button_pins: [],
  pwm_pins: [1, 2, 14, 15, 16], // @todo need to update
  adc_pins: [26, 27, 28],
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
  get LED0() {
    if (!this._led0) {
      var LED = global.require('led').LED;
      this._led0 = new LED(this.led_pins[0]);
    }
    return this._led0
  },
  get BTN0() {
    if (!this._btn0) {
      var Button = global.require('button').Button;
      this._btn0 = new Button(this.button_pins[0], FALLING);
    }
    return this._btn0;
  }
}
