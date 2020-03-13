var EventEmitter = require('events').EventEmitter;
var stream = require('stream');

/**
 * Socket class
 */
class Socket extends stream.Duplex {
  constructor () {
    super();
    this.localAddress = null;
    this.localPort = null;
    this.remoteAddress = null;
    this.remotePort = null;
    this._fd = -1;
    this._bindDev();
  }

  _bindDev () {
    if (!this._dev && global.__netdev) {
      this._dev = global.__netdev;
    }
  }
  
  _socket (fd) {
    this._bindDev();
    if (this._dev) {
      this._fd = fd;
      console.log("_socket 1");
      var sck = this._dev.get(this._fd);
      console.log("_socket 2");
      if (sck) {
        console.log("_socket 3");
        this.localAddress = sck.laddr;
        this.localPort = sck.lport;
        this.remoteAddress = sck.raddr;
        this.remotePort = sck.rport;      
        console.log("_socket 4");
        sck.connect_cb = () => {
          console.log("_socket 5");
          this.localAddress = sck.laddr;
          this.localPort = sck.lport;
          this.remoteAddress = sck.raddr;
          this.remotePort = sck.rport;
          this.emit('connect');
          this.emit('ready');        
        }
        console.log("_socket 6");
        sck.close_cb = () => { this._afterDestroy() }
        sck.read_cb = (data) => { this.push(data) }
        sck.shutdown_cb = () => { this._afterEnd() }
      }
    } else {
      console.log("_socket 7");
      throw new SystemError(6); // ENXIO
    }
  }

  /**
   * Initiates a connection
   * @param {object} options
   *   .host {string}
   *   .port {number}
   * @param {function} connectListener
   * @return {Socket}
   */
  connect (options, connectListener) {
    console.log("Socket.connect 1");
    this._bindDev();
    console.log("Socket.connect 2");
    if (this._dev) {
      console.log("Socket.connect 3");
      var fd = this._dev.socket(null, 'STREAM');    
      console.log("Socket.connect 4");
      if (connectListener) {
        console.log("Socket.connect 5");
        this.on('connect', connectListener);
      }
      console.log("Socket.connect 6");
      if (fd > -1) {
        console.log("Socket.connect 7");
        this._socket(fd);
        this._dev.connect(this._fd, options.host, options.port, (err) => {
          if (err) {
            this.emit('error', new SystemError(this._dev.errno));
          }
        });
      }
      console.log("Socket.connect 8");
    } else {
      console.log("Socket.connect 9");
      this.emit('error', new SystemError(6)); // ENXIO
    }
    console.log("Socket.connect 10");
    return this;
  }

  /**
   * @override
   * Destroy the connection
   * @param {function} cb
   */
  _destroy(cb) {
    this._bindDev();
    if (this._dev) {
      this._dev.close(this._fd, (err) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb();
        }
      })
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }
  
  /**
   * @override
   * Write data to the stream
   * @param {Uint8Array|string} chunk
   * @param {function} cb
   */
  _write (chunk, cb) {
    this._bindDev();
    if (this._dev) {
      this._dev.write(this._fd, chunk, (err) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno)); // eslint-disable-line
        } else {
          if (cb) cb();
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }

  /**
   * @override
   * Signal finish to write data
   * @param {function} cb
   */
  _final (cb) {
    this._bindDev();
    if (this._dev) {
      this._dev.shutdown(this._fd, 1, (err) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno)); // eslint-disable-line
        } else {
          if (cb) cb();
        }
      })
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }
}

/**
 * Server class
 */
class Server extends EventEmitter {
  constructor (options, connectionListener) {
    super();
    if (typeof options === 'function') {
      connectionListener = options;
      options = undefined;
    }
    if (connectionListener) {
      this.on('connection', connectionListener);
    }    
    this._fd = -1;
    this._bindDev();
  }

  _bindDev () {
    if (!this._dev && global.__netdev) {
      this._dev = global.__netdev;
    }
  }

  /**
   * Listen incoming connections
   * @param {number} port
   * @param {function} cb
   * @return {this}
   */
  listen (port, cb) {
    this._bindDev();
    if (this._dev) {
      this._fd = this._dev.socket(null, 'STREAM');
      if (this._fd < 0) {
        this.emit('error', new SystemError(this._dev.errno));
        return;
      } else {
        var sck = this._dev.get(this._fd);
        sck.accept_cb = (fd) => {
          var client = new Socket();
          client._socket(fd);
          this.emit('connection', client);
        }
      }
      this._dev.bind(this._fd, '127.0.0.1', port, (err) => {
        if (err) {
          this.emit('error', new SystemError(this._dev.errno));
        } else {
          this._dev.listen(this._fd, (err) => {
            if (err) {
              this.emit('error', new SystemError(this._dev.errno));
            } else {
              if (cb) this.on('listening', cb);
              this.emit('listening');
            }
          })
        }
      });
    } else {
      this.emit('error', new SystemError(6)); // ENXIO
    }
    return this;
  }

  /**
   * Close server
   * @param {function} cb
   * @return {this}
   */
  close (cb) {
    this._bindDev();
    if (this._dev) {
      this._dev.close(this._fd, (err) => {
        if (err) {
          this.emit('error', new SystemError(this._dev.errno))
        } else {
          this.emit('close');
          if (cb) cb();
        }
      });
    } else {
      this.emit('error', new SystemError(6)); // ENXIO
    }
    return this;
  }
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
}

exports.connect = exports.createConnection;

/**
 * Create a server
 * @param {object} options
 * @param {function} connectionListener
 */
exports.createServer = function (options, connectionListener) {
  var server = new Server(options, connectionListener);
  return server;
}
