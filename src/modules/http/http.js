var stream = require('stream');
var net = require('net');

class HTTPParser {
  constructor (incoming) {
    this._buffer = '';

    this.incoming = incoming;
    this.incoming.socket.on('data', (data) => this.process(data));
    this.incoming.socket.on('end', () => this.end());
    this.incoming.socket.on('close', () => this.end());
        
    this.method = null;
    this.statusCode = 0;
    this.statusMessage = null;
    this.httpVersion = null;
    this.url = null;
    this.headers = {};
    this.body = '';    
    this.headersComplete = false;
    this.onHeadersComplete = null;
    this.onComplete = null;
  }
  
  process (data) {
    this._buffer += data;
    if (this.headersComplete) {
      this.parse();
    } else {
      var idx = this._buffer.indexOf('\r\n\r\n'); // end of header
      if (idx > -1) {
        var head = this._buffer.substr(0, idx);
        var chunk = this._buffer.substr(idx + 4);
        this.parseHead(head);
        this.headersComplete = true;
        if (this.onHeadersComplete) this.onHeadersComplete();
        this.assignTo(this.incoming);
        this._buffer = chunk;
        this.parse();
      }
    }
  }
  
  end () {
    if (!this.incoming.complete) {
      this.incoming.complete = true;
      this.incoming.emit('end');
      if (this.onComplete) this.onComplete();
    }
  }
  
  parse () {
    if (this.headers['Transfer-Encoding'] === 'chunked') {
      this.parseBodyChunk();
    } else {
      this.parseBody();
    }
  }
  
  parseHead (head) {
    var ls = head.split('\n');
    var lt = ls[0].split(' ');
    if (lt[0].trim().startsWith('HTTP/')) { // status line
      this.httpVersion = lt[0].trim().split('/')[1];
      this.statusCode = parseInt(lt[1].trim());
      this.statusMessage = lt[2].trim();      
    } else { // request line
      this.method = lt[0].trim();
      this.url =  lt[1].trim();
      this.httpVersion = lt[2].trim().split('/')[1];
    }
    // headers
    for (var i = 1; i < ls.length; i++) {
      if (ls[i].includes(':')) {
        var ht = ls[i].split(':');
        this.headers[ht[0].trim()] = ht[1].trim();
      }
    }
  }
  
  parseBodyChunk () {
    var quit = false;
    while (!quit) {
      var idx = this._buffer.indexOf('\r\n');
      if (idx > -1) {
        var l = this._buffer.substr(0, idx);
        var cl = parseInt('0x' + l); // chunk length
        if (cl > 0) {
          if (this._buffer.length >= idx + cl + 2) { // chunk received
            var chunk = this._buffer.substr(idx + 2, cl)
            this.incoming.emit('data', chunk);
            this.body += chunk;
            this._buffer = this._buffer.substr(idx + 2 + cl + 2);
          } else { // data not received yet
            quit = true;
          }
        } else { // end chunk
          quit = true;
          this._buffer = '';
          this.end();
        }
      }
    }
  }
  
  parseBody () {
    var len = parseInt(this.headers['Content-Length'] || '0');
    if (this._buffer.length >= len) {
      this.body = this._buffer;
      this._buffer = '';
      this.incoming.emit('data', this.body);
      this.end();
    }
  }
  
  assignTo (incoming) {
    if (this.method) { // request
      incoming.httpVersion = this.httpVersion;
      incoming.method = this.method;
      incoming.url = this.url;
      Object.assign(incoming.headers, this.headers);      
    } else { // response
      incoming.httpVersion = this.httpVersion;
      incoming.statusCode = this.statusCode;
      incoming.statusMessage = this.statusMessage;
      Object.assign(incoming.headers, this.headers);
    }
  }
}


/*
Event: 'aborted'
Event: 'close'
message.aborted
message.complete
message.destroy([error])
message.headers
message.httpVersion
message.method
message.rawHeaders
message.rawTrailers
message.setTimeout(msecs[, callback])
message.socket
message.statusCode
message.statusMessage
message.trailers
message.url
*/

/**
 * IncomingMessage
 */
class IncomingMessage extends stream.Readable {
  constructor (socket) {
    super();
    this.headers = {};
    this.method = null;
    this.httpVersion = '1.1';
    this.statusCode = 0;
    this.statusMessage = '';
    this.url = null;
    this.complete = false;

    this.socket = socket;
    this.socket.on('close', () => this.emit('close'));    
  }
}

/**
 * OutgoingMessage class
 */
class OutgoingMessage extends stream.Writable {
  constructor () {
    super();
    this._buffer = '';
    this.headers = {};
  }

  setHeader (name, value) {
    this.headers[name] = value;
  }
  
  getHeader (name) {
    return this.headers[name];
  }
  
  removeHeader (name) {
    delete this.headers[name];
  }
  
  write (chunk, callback) {
    if (chunk) {
      if (this.headers['Transfer-Encoding'] === 'chunked') {
        this._buffer += chunk.length.toString(16) + '\r\n' + chunk + 'r\n';
      } else {
        this._buffer += chunk;
      }
    }
    if (callback) callback();
    return true;
  }
  
  end (chunk, callback) {
    if (chunk) this.write(chunk);
    if (this.headers['Transfer-Encoding'] === 'chunked') {
      this._buffer += '0\r\n\r\n';
    } else {
      this.headers['Content-Length'] = this._buffer.length;
    }
    if (callback) callback();
    return true;
  }
}

/**
 * ClientRequest class
 */

/*
Event: 'abort'
Event: 'connect'
Event: 'continue'
Event: 'information'
Event: 'response'
Event: 'socket'
Event: 'timeout'
Event: 'upgrade'
request.abort()
request.aborted
request.connection
request.end([data[, encoding]][, callback])
request.finished
request.flushHeaders()
request.getHeader(name)
request.maxHeadersCount
request.path
request.removeHeader(name)
request.setHeader(name, value)
request.setNoDelay([noDelay])
request.setSocketKeepAlive([enable][, initialDelay])
request.setTimeout(timeout[, callback])
request.socket
request.writableEnded
request.writableFinished
request.write(chunk[, encoding][, callback])
*/
class ClientRequest extends OutgoingMessage {
  constructor (options) {
    super();    
    this.method = options.method || 'GET';
    this.path = options.path || '/';
    this.host = options.host;
    this.port = options.port;

    if (options.headers) Object.assign(this.headers, options.headers);

    this.socket = new net.Socket();
    this.incoming = new IncomingMessage(this.socket);
    this._parser = new HTTPParser(this.incoming);
    this._parser.onHeadersComplete = () => {
      this.emit('response', this.incoming);
    }
  }
    
  abort () {
    
  }

  write (chunk, callback) {
    this._buffer += chunk;
    if (callback) callback();
  }

  end (chunk, callback) {
    if (chunk) {
      this._buffer += chunk;
    }

    // make http request message
    var msg = `${this.method} ${this.path} HTTP/1.1\r\n`;
    this.headers['Content-Length'] = this._buffer.length;
    for (var key in this.headers) {
      msg += `${key}: ${this.headers[key]}\r\n`;
    }
    msg += '\r\n'; // end of header
    msg += this._buffer;

    this.socket.connect({ host: this.host, port: this.port }, () => {
      // this.emit('connect', this.incomingMessage, this.socket);
      this.socket.write(msg, callback);
      this._buffer = '';
    });
  }
}


/**
 * ServerResponse class
 */
class ServerResponse extends OutgoingMessage {
  constructor (socket) {
    super();
    this.socket = socket;
    this.statusCode = 200;
    this.statusMessage = 'OK';
    this.headersSent = false;
    // event: 'end'
    // event: 'close'
    // event: 'finish'
    this.socket.on('close', () => this.emit('close'));
  }
  
  writeHead (statusCode, statusMessage, headers) {
    this.statusCode = statusCode;
    if (statusMessage) this.statusMessage = statusMessage;
    if (headers) Object.assign(this.headers, headers);
    if (!this.headers['Content-Length']) { // chunked transfer mode
      this.headers['Transfer-Encoding'] = 'chunked';
    }
    var msg = `HTTP/1.1 ${this.statusCode} ${this.statusMessage}\r\n`;
    for (var name in this.headers) {
      msg += `${name}: ${this.headers[name]}\r\n`;
    }
    msg += '\r\n'; // end of header
    this._buffer += msg;
    this.headersSent = true;
    return this;
  }
  
  write (data, callback) {
    if (!this.headersSent) {
      this.writeHead(200);
    }
    return super.write(data, callback);
  }
  
  end (data, callback) {
    if (!this.headersSent) {
      this.writeHead(200);
    }
    super.end(data, callback);
    
    this.socket.write(this._buffer, () => {
      // this.socket.end();
    })
    
    return this;
  }
}

/**
 * Server
 */
class Server extends net.Server {
  constructor () {
    super();
    this.on('connection', (socket) => this.handleConnection(socket));
  }
  
  handleConnection (socket) {    
    var req = new IncomingMessage(socket);
    var res = new ServerResponse(socket);
    var parser = new HTTPParser(req);
    parser.onHeadersComplete = () => {
      this.emit('request', req, res);
    }
  }
}

exports.request = function (options, callback) {
  var req = new ClientRequest(options);
  if (callback) req.once('response', callback);
  return req;
}

exports.createServer = function (requestListener) {
  var server = new Server();
  if (requestListener) server.on('request', requestListener);
  return server;
}
