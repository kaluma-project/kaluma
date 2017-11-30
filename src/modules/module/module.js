
function Module(id) {
  this.id = id;
  this.exports = {};
}

Module.cache = {}

Module.require = function (id) {
  if (Module.cache[id]) {
    return Module.cache[id].exports;
  }
  if (process.natives[id]) {
    var mod = new Module(id);
    Module.cache[id] = mod;
    mod.loadNative();
    return mod.exports;
  } else {
    print('Try to load non-native module: ' + id);
  }
}

Module.prototype.loadNative = function () {
  var wrapper = process.getNative(this.id);
  wrapper(this.exports, Module.require, this);
}

module.exports = Module;
