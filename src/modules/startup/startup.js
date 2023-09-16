/**
 * Module class
 */

function Module(id) {
  this.id = id;
  this.exports = {};
}

Module.cache = {};

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
  throw new Error("Failed to load module: " + id);
};

Module.prototype.loadBuiltin = function () {
  var fn = process.getBuiltinModule(this.id);
  fn(this.exports, Module.require, this);
};

global.require = Module.require;

/**
 * Storage object
 */

if (process.builtin_modules.indexOf("storage") > -1) {
  Object.defineProperty(global, "storage", {
    get: function () {
      return Module.require("storage");
    },
  });
}

/**
 * Board object
 */

global.board = {
  gpio: function (...args) {
    var GPIO = global.require("gpio").GPIO;
    return new GPIO(...args);
  },
  led: function (...args) {
    var LED = global.require("led").LED;
    return new LED(...args);
  },
  button: function (...args) {
    var Button = global.require("button").Button;
    return new Button(...args);
  },
  pwm: function (...args) {
    var PWM = global.require("pwm").PWM;
    return new PWM(...args);
  },
  adc: function (...args) {
    var ADC = global.require("adc").ADC;
    return new ADC(...args);
  },
  i2c: function (...args) {
    var I2C = global.require("i2c").I2C;
    return new I2C(...args);
  },
  spi: function (...args) {
    var SPI = global.require("spi").SPI;
    return new SPI(...args);
  },
  uart: function (...args) {
    var UART = global.require("uart").UART;
    return new UART(...args);
  }
};
