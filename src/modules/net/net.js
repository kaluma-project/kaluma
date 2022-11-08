const net_native = process.binding(process.binding.net);
const { EventEmitter } = require("events");
const { Stream } = require("stream");

/**
 * Socket class
 */
class Socket extends Stream {
  constructor() {
    super();
    this._native = new net_native.Socket();
  }

  /**
   * Initiates a connection
   * @param {object} options
   *   .host {string}
   *   .port {number}
   * @param {function} connectListener
   * @return {Socket}
   */
  connect(options, connectListener) {
    this._native.connect(options);
  }
}

/**
 * Server class
 */
class Server extends EventEmitter {
  constructor(connectionListener) {
    super();
  }

  /**
   * Listen incoming connections
   * @param {number} port
   * @param {function} cb
   * @return {this}
   */
  listen(port, cb) {}

  /**
   * Close server
   * @param {function} cb
   * @return {this}
   */
  close(cb) {}
}

exports.Socket = Socket;
exports.Server = Server;

/**
 * Create a socket and initiate connection
 * @param {object} options
 * @param {function} connectListener
 */
exports.createConnection = function (options, connectListener) {
  var client = new Socket();
  if (connectListener) {
    client.connect(options, connectListener);
  }
  return client;
};

exports.connect = exports.createConnection;

/**
 * Create a server
 * @param {object} options
 * @param {function} connectionListener
 */
exports.createServer = function (options, connectionListener) {
  var server = new Server(options, connectionListener);
  return server;
};
