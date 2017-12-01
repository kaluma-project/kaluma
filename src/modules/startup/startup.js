
function Module(id) {
  this.id = id;
  this.exports = {};
}

Module.cache = {}

Module.require = function (id) {
  if (Module.cache[id]) {
    return Module.cache[id].exports;
  }
  if (process.native_modules.indexOf(id) >= 0) {
    var mod = new Module(id);
    Module.cache[id] = mod;
    mod.loadNative();
    return mod.exports;
  } else {
    print('Try to load non-native module: ' + id);
  }
}

Module.prototype.loadNative = function () {
  var fn = process.getNativeModule(this.id);
  fn(this.exports, Module.require, this);
}

var timers = Module.require('timers');

global.require = Module.require;
global.setTimeout = timers.setTimeout;
global.setInterval = timers.setInterval;
