var EventEmitter = require('events').EventEmitter;

/**
 * Astract stream class
 */
class Stream extends EventEmitter {
  constructor () {
    super();
    this.destroyed = false;    
  }

  _destroy (cb) {
    if (cb) cb();
  }
  
  _onDestroy() {
    if (!this.destroyed) {
      this.destroyed = true;
      this.emit('close');
    }    
  }
  
  /**
   * Destroy the stream
   */
  destroy () {
    this._destroy(() => {
      this._onDestroy();
    })
    return this;
  }  
}

/**
 * Readable class
 */
class Readable extends Stream {
  constructor () {
    super();
    this._rbuf = '';
    this.readableFlowing = true;
    this.readableEnded = false;
    this.readable = false;
  }

  _onEnd () {
    if (!this.readableEnded) {
      this.readableEnded = true;
      this.readableFlowing = false;
      this.readable = false;
      this._rbuf = '';
      this.emit('end');
    }
  }

  _onDestroy () {
    this._onEnd();
    super._onDestroy();
  }

  /**
   * Return whether stream is paused or not
   * @return {boolean}
   */
  isPaused () {
    return !this.readableFlowing;
  }
  
  /**
   * Pause the stream
   */
  pause () {
    this.readableFlowing = false;
    this.emit('pause');
  }
  
  /**
   * Resume the stream
   */
  resume () {
    this.readableFlowing = true;
    this.emit('resume');
  }

  /**
   * @protected
   * Push a chunk of data to this readable stream.
   * This method is only allowed to the descendants.
   * 
   * @param {string} chunk
   */
  push (chunk) {
    if (this.readableFlowing && this.listenerCount('data') < 1) {
      this.readableFlowing = false;
    }
    if (this.readableFlowing) {
      this._rbuf += chunk;
      this.emit('data', this._rbuf);
      this._rbuf = '';
      this.readable = false;
    } else {
      this._rbuf += chunk;
      this.readable = true;
      this.emit('readable');      
    }
  }
  
  /**
   * Read data
   * 
   * @param {number} size
   * @return {string}
   */
  read (size) {
    var chunk = null;
    if (this._rbuf.length > 0) {
      if (size > 0) {
        chunk = this._rbuf.substr(0, size);
        this._rbuf = this._rbuf.substr(size);
      } else {
        chunk = this._rbuf;
        this._rbuf = '';
      }
    }
    if (this._rbuf.length === 0) {
      this.readable = false;
    }
    return chunk;
  }
}

/**
 * Writable
 */
class Writable extends Stream {
  constructor () {
    super();
    this._wbuf = ''
    this.writable = true;
    this.writableFinished = false;
    this.destroyed = false;
  }

  _onDestroy () {
    if (!this.destroyed) {
      this._wbuf = '';
      this.writable = false;
      this.writableFinished = true;
      this.destroyed = true;
      this.emit('close');
    }
  }

  /**
   * @protected
   * @param {string} chunk 
   * @param {Function} cb 
   */
  _write (chunk, cb) {}

  /**
   * @protected
   * @param {Function} cb
   */
  _finish (cb) {}
    
  /**
   * @abstract
   * Abstrct method for writing a chunk of data to the stream
   * 
   * @param {string} chunk
   * @param {Function} callback
   * @return {boolean}
   */
  write (chunk, callback) {
    if (this.writable) {
      this.writable = false;
      this._write(chunk, () => {
        this.afterWrite(callback);
      })
    } else {
      this._wbuf += chunk;
    }
    return this.writable;
  }

  /**
   * Signals that no more data to write.
   * 
   * @param {string} chunk
   * @param {Function} callback
   * @return {Writable}
   */
  end (chunk, callback) {
    if (typeof chunk === 'function') {
      chunk = undefined;
      callback = chunk;
    }
    if (callback) {
      this.once('finish', callback);
    }
    if (chunk) {
      var done = this.write(chunk, () => {
        if (!this.writableFinished) this.finish();
      })
      if (done) {
        this.finish();
      }
    } else {
      this.finish();
    }
    return this;
  }
  
  afterWrite (callback) {
    if (this._wbuf.length > 0) {
      setTimeout(() => {
        this.write(this._wbuf, callback);
      }, 0);
    } else {
      this.writable = true;
      if (callback) callback();
      this.emit('drain');
    }
  }
  
  /**
   * @projected
   * Signal finish.
   */
  finish () {
    this._finish(() => {
      this.writableFinished = true;
      this.writable = false;
      this.emit('finish');          
    })
  }  
}

/**
 * Duplex class
 */
class Duplex extends Readable {
  constructor () {
    super();
    this._wbuf = ''
    this.writable = true;
    this.writableFinished = false;
  }
  
  _onDestroy () {
    this._onEnd();
    if (!this.destroyed) {
      this._wbuf = '';
      this.writable = false;
      this.writableFinished = true;
      this.destroyed = true;
      this.emit('close');
    }
  }
}

Duplex.prototype.write = Writable.prototype.write;
Duplex.prototype.end = Writable.prototype.end;
Duplex.prototype.afterWrite = Writable.prototype.afterWrite;
Duplex.prototype.finish = Writable.prototype.finish;

exports.Readable = Readable;
exports.Writable = Writable;
exports.Duplex = Duplex;
