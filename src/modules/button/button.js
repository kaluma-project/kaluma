var EventEmitter = require('events').EventEmitter;

/**
 * URL class.
 * @param {string} input
 */
class Button extends EventEmitter {
  constructor(pin, options) {
    super();
    options = options || {};
    this.watchId = -1;
    this.pin = pin;
    this.mode = (typeof options.mode === 'number') ? options.mode : INPUT_PULLUP;
    this.event = (typeof options.event === 'number') ? options.event : FALLING;
    this.debounce = (typeof options.debounce === 'number') ? options.debounce : 50;
    pinMode(this.pin, this.mode);
    this.watchId = setWatch(() => {
      this.emit('click');
    }, this.pin, this.event, this.debounce);
  }

  read() {
    return digitalRead(this.pin);
  }

  close() {
    clearWatch(this.watchId);
  }
}

exports.Button = Button;
