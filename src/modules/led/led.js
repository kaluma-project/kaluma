function LED (pin) {
  this.pin = pin;
  pinMode(this.pin, OUTPUT);
}

LED.prototype.on = function () {
  digitalWrite(this.pin, 1)
}

LED.prototype.off = function () {
  digitalWrite(this.pin, 0)
}

LED.prototype.toggle = function () {
  digitalToggle(this.pin)
}

LED.prototype.read = function () {
  return digitalRead(this.pin)
}

module.exports = LED;
