var EventEmitter = require('events').EventEmitter;

function Button (pin, event, debounce) {
  EventEmitter.call(this);
  this.pin = pin;
  pinMode(this.pin, INPUT);
  this.mode = event;
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
