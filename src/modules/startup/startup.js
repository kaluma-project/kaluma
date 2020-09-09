/**
 * Module class
 */

function Module(id) {
  this.id = id;
  this.exports = {};
}

Module.cache = {}

Module.require = function (id) {
  if (Module.cache[id]) {
    return Module.cache[id].exports;
  }
  if (process.builtin_modules.indexOf(id) >= 0) {
    var mod = new Module(id);
    mod.loadBuiltin();
    Module.cache[id] = mod;
    return mod.exports;
  }
  throw new Error('Failed to load module: ' + id);
}

Module.prototype.loadBuiltin = function () {
  var fn = process.getBuiltinModule(this.id);
  fn(this.exports, Module.require, this);
}

class SystemError extends Error {
  constructor(errno, code, message) {
    super(message);
    this.name = 'SystemError';
    this.errno = errno;
    this.code = code;
    this.message = `(errno:${this.errno})`;
  }
}

global.require = Module.require;
global.SystemError = SystemError;

/**
 * Storage object
 */
if (process.builtin_modules.indexOf('storage') > -1) {
  var Storage = Module.require('storage').Storage;
  global.storage = new Storage();
}

/**
 * Board object
 */

if (process.builtin_modules.indexOf('gpio') > -1) {
  board.gpio = function (pin, mode) {
    var GPIO = Module.require('gpio');
    return new GPIO(pin, mode);
  }
}

if (process.builtin_modules.indexOf('led') > -1) {
  board.led = function (pin) {
    var LED = Module.require('led');
    return new LED(pin);
  }
}

/**
 * PWM object
 */
if (process.builtin_modules.indexOf('pwm') > -1) {
    var PWM = Module.require('pwm').PWM;
    global.pwm = PWM;
}
