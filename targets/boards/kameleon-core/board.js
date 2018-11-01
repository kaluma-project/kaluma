global.board = {
  name: 'kameleon-core',
  NUM_GPIO: 22,
  NUM_LED: 1,
  NUM_BUTTON: 1,
  NUM_PWM: 6,
  NUM_ADC: 7,
  NUM_I2C: 2,
  NUM_SPI: 2,
  NUM_UART: 2,
  led_pins: [20],
  button_pins: [21],
  pwm_pins: [8, 9, 13, 14, 15, 16],
  adc_pins: [2, 3, 4, 5, 10, 11, 12],
  gpio: function (pin, mode) {
    var GPIO = global.require('gpio');
    return new GPIO(pin, mode);
  },
  led: function (pin) {
    var LED = global.require('led');
    return new LED(pin);
  },
  button: function (pin, pull, debounce) {
    var Button = global.require('button');
    return new Button(pin, pull, debounce);
  },
  pwm: function (pin) {
    if (this.pwm_pins.indexOf(pin) < 0) {
      throw Error('The pin is not PWM capable.');
    }
    var PWM = global.require('pwm');
    return new PWM(pin);
  },
  get LED0() {
    if (!this._led0) {
      var LED = global.require('led');
      this._led0 = new LED(this.led_pins[0]);
    }
    return this._led0
  },
  get BTN0() {
    if (!this._btn0) {
      var Button = global.require('button');
      this._btn0 = new Button(this.button_pins[0], PULL_UP);
    }
    return this._btn0;
  }
}
