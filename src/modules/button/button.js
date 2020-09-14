var EventEmitter = require('events').EventEmitter;

function Button (pin, event, debounce, int_pull) {
  EventEmitter.call(this);
  this.pin = pin;
  this.int_pull = (typeof int_pull === 'number' ? int_pull : PULL_NO);
  if ((this.int_pull < PULL_NO) || (this.int_pull > PULL_UP)) {
    this.int_pull = PULL_NO;
  }
  if (this.int_pull == PULL_UP) {
    pinMode(this.pin, INPUT_PULLUP);
  } else if (this.int_pull == PULL_DOWN) {
    pinMode(this.pin, INPUT_PULLDOWN);
  } else {
    pinMode(this.pin, INPUT);
  }
  this.mode = (typeof event === 'number' ? event : FALLING);
  if ((this.mode < FALLING) || (this.mode > CHANGE)) {
    this.mode = FALLING;
  }
  this.debounce = (typeof debounce === 'number' ? debounce : 50);
  var self = this;
  this.watchId = setWatch(function () {
    self.emit('click');
  }, this.pin, this.mode, this.debounce);
}

/* Inherits from EventEmitter */
Button.prototype = new EventEmitter();
Button.prototype.constructor = Button;

Button.prototype.read = function () {
  return digitalRead(this.pin);
}

Button.prototype.close = function () {
  clearWatch(this.watchId);
}

exports.Button = Button;
