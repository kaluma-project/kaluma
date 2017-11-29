print("this is startup.js...");

function test_snapshot() {
  print("snapshot loaded successfully");
}

function NativeModule(id) {
  this.id = id;
  this.exports = {};
}

NativeModule.cache = {}

NativeModule.require = function (id) {
  if (NativeModule.cache[id]) {
    return NativeModule.cache[id].exports;
  }
  var mod = new NativeModule(id);
  NativeModule.cache[id] = mod;
  mod.compile();
  return mod.exports;
}

NativeModule.prototype.compile = function () {
  var wrapper = process.getNative(this.id);
  wrapper(this.exports, NativeModule.require, this);
}
