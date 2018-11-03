function ADC (pin) {
  this.pin = pin;
}

ADC.prototype.read = function () {
  return analogRead(this.pin)
}

module.exports = ADC;
