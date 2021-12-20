const { EventEmitter } = require("events");

/**
 * Stream class
 */
class Stream extends EventEmitter {
  constructor() {
    super();
  }
  readable() {}
  read(length) {}
  writable() {}
  write(data) {}
  end() {}
  destroy() {}
}

exports.Stream = Stream;
