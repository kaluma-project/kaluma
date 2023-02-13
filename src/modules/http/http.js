var stream = require('stream');
var net = require('net');

/**
 * HTTPParser class
 * @param {IncomingMessage} incoming
 */
class HTTPParser {
  constructor(incoming) {
    this._buf = '';
    this.incoming = incoming;
    this.incoming.socket.on('data', (chunk) => { this.push(chunk) });
    this.incoming.socket.on('end', () => { this.end() });
    this.incoming.socket.on('close', () => { this.end() });
    this.body = '';
    this.headersComplete = false;
    this.onHeadersComplete = null;
    this.onComplete = null;
  }

  /**
   * Push a chunk of data to buffer
   * @param {Uint8Array|string} chunk
   */
  push(chunk) {
    if (chunk instanceof Uint8Array) {
      this._buf += String.fromCharCode.apply(null, chunk);
    } else {
      this._buf += chunk;
    }
    if (this.headersComplete) {
      this.parseBody();
    } else {
      var idx = this._buf.indexOf('\r\n\r\n'); // end of header
      if (idx > -1) {
        var head = this._buf.substr(0, idx);
        var body = this._buf.substr(idx + 4);
        this._buf = body;
        this.parseHead(head);
        this.headersComplete = true;
        if (this.onHeadersComplete) this.onHeadersComplete();
        this.parseBody();
      }
    }
  }

  /**
   * Parse HTTP header and assign to incoming message
   * @param {string} head
   */
  parseHead(head) {
    var ls = head.split('\n');
    var lt = ls[0].split(' ');
    if (lt[0].trim().startsWith('HTTP/')) { // http response
      this.incoming.httpVersion = lt[0].trim().split('/')[1];
      this.incoming.statusCode = parseInt(lt[1].trim());
      this.incoming.statusMessage = lt[2].trim();
    } else { // http request
      this.incoming.method = lt[0].trim();
      this.incoming.url = lt[1].trim();
      this.incoming.httpVersion = lt[2].trim().split('/')[1];
    }
    // headers
    for (var i = 1; i < ls.length; i++) {
      if (ls[i].includes(':')) {
        var ht = ls[i].split(':');
        this.incoming.headers[ht[0].trim().toLowerCase()] = ht[1].trim();
      }
    }
  }

  /**
   * Parse HTTP body
   */
  parseBody() {
    if (this.incoming.headers['transfer-encoding'] === 'chunked') {
      var quit = false;
      while (!quit) {
        var idx = this._buf.indexOf('\r\n');
        if (idx > -1) {
          var l = this._buf.substr(0, idx);
          var cl = parseInt('0x' + l); // chunk length
          if (cl > 0) {
            if (this._buf.length >= idx + cl + 2) { // chunk received
              var chunk = this._buf.substr(idx + 2, cl)
              var encoder = new TextEncoder('ascii');
              this.incoming.push(encoder.encode(chunk));
              this.body += chunk;
              this._buf = this._buf.substr(idx + 2 + cl + 2);
            } else { // data not received yet
              quit = true;
            }
          } else { // end chunk
            quit = true;
            this._buf = '';
            this.end();
          }
        }
      }
    } else {
      var len = parseInt(this.incoming.headers['Content-Length'] || '0');
      if (this._buf.length >= len) {
        this.body = this._buf;
        this._buf = '';
        var encoder = new TextEncoder('ascii');
        this.incoming.push(encoder.encode(this.body));
        this.end();
      }
    }
  }

  /**
   * Finishes to parse incoming message.
   * Emits 'end' event to the incoming message.
   */
  end() {
    if (!this.incoming.complete) {
      if (this._buf.length > 0) {
        var encoder = new TextEncoder('ascii');
        this.incoming.push(encoder.encode(this._buf));
        this._buf = '';
      }
      this.incoming.complete = true;
      this.incoming._afterEnd();
      if (this.onComplete) this.onComplete();
    }
  }
}

/**
 * IncomingMessage
 */
class IncomingMessage extends stream.Readable {
  constructor(socket) {
    super();
    this.headers = {};
    this.method = null;
    this.httpVersion = '1.1';
    this.statusCode = 0;
    this.statusMessage = '';
    this.url = null;
    this.complete = false;
    this.socket = socket;
    this.socket.on('close', () => {
      this._afterDestroy();
    });
  }
}

/**
 * @protected
 * OutgoingMessage class
 */
class OutgoingMessage extends stream.Writable {
  constructor(socket) {
    super();
    this.socket = socket;
    this.headers = {};
    this.headersSent = false;
    this.socket.on('close', () => {
      this._afterFinish();
      this._afterDestroy();
    });
  }

  /**
   * Check is transfer-encoding is chunked
   * @return {boolean}
   */
  _isTransferChunked() {
    return (this.headers['transfer-encoding'] === 'chunked');
  }

  /**
   * Encode chunk
   * @param {Uint8Array|string} chunk
   * @return {string}
   */
  _encodeChunk(chunk) {
    if (chunk instanceof Uint8Array)
      chunk = String.fromCharCode.apply(null, chunk);
    return chunk.length.toString(16) + '\r\n' + chunk + '\r\n';
  }

  /**
   * @override
   */
  _destroy(cb) {
    this.socket.destroy(cb);
  }

  /**
   * @override
   */
  _write(chunk, cb) {
    this.socket.write(chunk, cb);
  }

  /**
   * @override
   */
  _final(cb) {
    this.socket.end(cb);
  }

  /**
   * Set header value
   * @param {string} name 
   * @param {string} value 
   */
  setHeader(name, value) {
    this.headers[name.toLowerCase()] = value;
  }

  /**
   * get header value
   * @param {string} name 
   * @return {string}
   */
  getHeader(name) {
    return this.headers[name.toLowerCase()];
  }

  /**
   * Remove a header value
   * @param {string} name 
   */
  removeHeader(name) {
    delete this.headers[name.toLowerCase()];
  }
}

/**
 * ClientRequest class
 */
class ClientRequest extends OutgoingMessage {
  constructor(options, socket) {
    super(socket);
    this.options = options;
    this.path = options.path || '/';
    if (this.options.headers) {
      Object.assign(this.headers, this.options.headers);
    }
    this.incoming = new IncomingMessage(this.socket);
    this._parser = new HTTPParser(this.incoming);
    this._parser.onHeadersComplete = () => {
      this.emit('response', this.incoming);
    }
    this.socket.on('error', err => {
      this.emit('error', err);
    });
  }

  /**
   * Flush headers to buffer.
   */
  flushHeaders() {
    if (!this.headers.hasOwnProperty['Content-Length']) {
      this.setHeader('transfer-encoding', 'chunked');
    }
    this._wbuf += `${this.options.method} ${this.path} HTTP/1.1\r\n`;
    for (var key in this.headers) {
      this._wbuf += `${key}: ${this.headers[key]}\r\n`;
    }
    this._wbuf += '\r\n'; // end of header
  }

  /**
   * @override
   * Write data on the stream
   * @param {Uint8Array|string} chunk 
   * @param {Function} cb
   * @return {this}
   */
  write(chunk, cb) {
    if (!this.headersSent) {
      this.flushHeaders();
      this.headersSent = true;
    }
    if (chunk) {
      if (chunk instanceof Uint8Array)
        chunk = String.fromCharCode.apply(null, chunk);
      if (this._isTransferChunked()) {
        this._wbuf += this._encodeChunk(chunk);
      } else {
        this._wbuf += chunk;
      }
    }
    if (cb) cb();
    return this;
  }

  /**
   * @override
   * Finish to write data on the stream
   * @param {Uint8Array|string} chunk
   * @param {Function} cb
   * @return {this}
   */
  end(chunk, cb) {
    if (!this.headersSent) {
      this.flushHeaders();
      this.headersSent = true;
    }
    if (chunk) {
      if (chunk instanceof Uint8Array)
        chunk = String.fromCharCode.apply(null, chunk);
      if (this._isTransferChunked()) {
        this._wbuf += this._encodeChunk(chunk);
      } else {
        this._wbuf += chunk;
      }
    }
    this.socket.connect(this.options, () => {
      var last = (this._isTransferChunked() ? '0\r\n\r\n' : undefined); // end of body
      super.end(last, cb);
      this._afterFinish();
    })
    return this;
  }
}


/**
 * ServerResponse class
 */
class ServerResponse extends OutgoingMessage {
  constructor(socket) {
    super(socket);
    this.statusCode = 200;
    this.statusMessage = 'OK';
  }

  /**
   * Write response headers and sent to client
   * @param {number} statusCode
   * @param {string} statusMessage
   * @param {Object} headers
   * @return {this}
   */
  writeHead(statusCode, statusMessage, headers) {
    if (!this.headersSent) {
      this.statusCode = statusCode;
      if (statusMessage) this.statusMessage = statusMessage;
      if (headers) Object.assign(this.headers, headers);
      if (!this.headers.hasOwnProperty['Content-Length']) { // chunked transfer mode
        this.headers['transfer-encoding'] = 'chunked';
      }
      var msg = `HTTP/1.1 ${this.statusCode} ${this.statusMessage}\r\n`;
      for (var name in this.headers) {
        msg += `${name}: ${this.headers[name]}\r\n`;
      }
      msg += '\r\n'; // end of header
      super.write(msg, () => {
        this.headersSent = true;
      });
    }
    return this;
  }

  /**
   * @override
   */
  write(chunk, cb) {
    if (!this.headersSent) {
      this.writeHead(200);
    }
    if (chunk && this._isTransferChunked()) {
      chunk = this._encodeChunk(chunk);
    }
    super.write(chunk, cb);
    return this;
  }

  /**
   * @override
   */
  end(chunk, cb) {
    if (!this.headersSent) {
      this.writeHead(200);
    }
    if (this._isTransferChunked()) {
      if (chunk) {
        chunk = this._encodeChunk(chunk, true) + '0\r\n\r\n'; // end of body
      } else {
        chunk = '0\r\n\r\n'; // end of body
      }
    }
    super.end(chunk, cb);
    this._afterFinish();
    return this;
  }
}

/**
 * Server
 */
class Server extends net.Server {
  constructor() {
    super();
    this.on('connection', (socket) => {
      var req = new IncomingMessage(socket);
      var parser = new HTTPParser(req);
      parser.onHeadersComplete = () => {
        var res = new ServerResponse(socket);
        this.emit('request', req, res);
      }
    });
  }
}

/**
 * HTTP request
 * @param {object} options
 *   .host {string}
 *   .port {number}
 *   .method {string}
 *   .path {string}
 *   .headers {object}
 * @param {Function} cb
 */
exports.request = function (options, cb) {
  var socket = new net.Socket();
  options.port = options.port || 80;
  options.method = options.method || 'GET';
  var req = new ClientRequest(options, socket);
  if (cb) req.once('response', cb);
  return req;
}

/**
 * HTTP GET request
 * @param {object} options
 *   .host {string}
 *   .port {number}
 *   .path {string}
 *   .headers {object}
* @param {Function} cb
 */
exports.get = function (options, cb) {
  var socket = new net.Socket();
  options.port = options.port || 80;
  options.method = 'GET';
  var req = new ClientRequest(options, socket);
  if (cb) req.once('response', cb);
  req.end();
  return req;
}

/**
 * Create a HTTP Server
 * @param {Function} requestListener
 */
exports.createServer = function (requestListener) {
  var server = new Server();
  if (requestListener) server.on('request', requestListener);
  return server;
}
