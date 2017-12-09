
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
    Module.cache[id] = mod;
    mod.loadBuiltin();
    return mod.exports;
  } else {
    print('Try to load non-native module: ' + id);
  }
}

Module.prototype.loadBuiltin = function () {
  var fn = process.getBuiltinModule(this.id);
  fn(this.exports, Module.require, this);
}

var timers = Module.require('timers');
var gpio = Module.require('gpio');

global.require = Module.require;
global.setTimeout = timers.setTimeout;
global.setInterval = timers.setInterval;
global.clearTimeout = timers.clearTimeout;
global.clearInterval = timers.clearInterval;
global.delay = timers.delay;
global.millis = timers.millis;

global.console = Module.require('console');

global.HIGH = gpio.HIGH;
global.LOW = gpio.LOW;
global.INPUT = gpio.INPUT;
global.OUTPUT = gpio.OUTPUT;
global.pinMode = gpio.pinMode;
global.digitalRead = gpio.digitalRead;
global.digitalWrite = gpio.digitalWrite;
global.digitalToggle = gpio.digitalToggle;
