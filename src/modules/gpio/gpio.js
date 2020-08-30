function GPIO (pin, mode) {
  this.pin = pin;
  if ((mode < INPUT) || (mode > INPUT_PULLUP))
    mode = INPUT;
  this.setMode(mode)
}

GPIO.prototype.read = function () {
  return digitalRead(this.pin)
}

GPIO.prototype.write = function (value) {
  digitalWrite(this.pin, value)
}

GPIO.prototype.toggle = function () {
  digitalToggle(this.pin)
}

GPIO.prototype.low = function () {
  digitalWrite(this.pin, 0)
}

GPIO.prototype.high = function () {
  digitalWrite(this.pin, 1)
}

GPIO.prototype.setMode = function (mode) {
  this.mode = typeof mode === 'number' ? mode : INPUT
  pinMode(this.pin, this.mode);
}

exports.GPIO = GPIO;
