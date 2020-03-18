var EventEmitter = require('events').EventEmitter;

/**
 * Astract stream class
 */
class Stream extends EventEmitter {
  constructor () {
    super();
    this.destroyed = false;
  }
  
  /**
   * @protected
   * @abstract
   * Implement how to destroy the stream
   * @param {Function} callback
   */  
  _destroy (callback) {} // eslint-disable-line

  /**
   * @protected
   * Should be called when after destroyed
   */
  _afterDestroy () {
    if (!this.destroyed) {
      this.destroyed = true;
      this.emit('close');
    }
  }
  
  /**
   * Destroy the stream
   * @return {this}
   */  
  destroy () {
    if (!this.destroyed) {
      this._destroy((err) => {
        if (err) {
          this.emit('error', err);
        } else {
          this._afterDestroy();
        }
      });      
    }
  }
}

/**
 * Readable class
 */
class Readable extends Stream {
  constructor () {
    super();
    this.readableEnded = false;
  }
  
  /**
   * @protected
   * Should be called when there is no more data to read.
   */
  _afterEnd () {
    if (!this.readableEnded) {
      this.readableEnded = true;
      this.emit('end');
    }
  }

  /**
   * @protected
   * Push a chunk of data to this readable stream.
   * @param {string} chunk
   * @return {this}
   */  
  push (chunk) {
    this.emit('data', chunk);
    return this;
  }
}

/**
 * Writable class
 */
class Writable extends Stream {
  constructor () {
    super();
    this._wbuf = '';
    this.writableEnded = false;
    this.writableFinished = false;
  }

  /**
   * @protected
   * @abstract
   * Implement how to write data on the stream
   * @param {Function} callback
   */
  _write (data, callback) {} // eslint-disable-line

  /**
   * @protected
   * @abstract
   * Implement how to finish to write on the stream
   * @param {Function} callback
   */
  _final (callback) {} // eslint-disable-line
  
  /**
   * @protected
   * Should be called when after finished
   */
  _afterFinish () {
    if (!this.writableFinished) {
      this.emit('finish');
      this.writableFinished = true;
    }
  }
  
  /**
   * Write a chunk of data to the stream
   * @param {string} chunk
   * @param {Function} callback
   * @return {boolean}
   */
  write (chunk, callback) {
    if (!this.writableEnded) {
      if (chunk) {
        this._wbuf += chunk;
      }
      setTimeout(() => this.flush(), 0);
      if (callback) callback();
    }
    return this._wbuf.length === 0;
  }
  
  /**
   * Finish to write on the stream.
   * @param {string} chunk
   * @param {Function} callback
   * @return {Writable}
   */  
  end (chunk, callback) {
    if (typeof chunk === 'function') {
      callback = chunk;
      chunk = undefined;
    }
    if (chunk) {
      this._wbuf += chunk;
    }
    if (callback) {
      this.once('finish', callback);
    }
    this.writableEnded = true;
    if (this._wbuf.length > 0) {
      this.flush();
    } else {
      this.finish();
    }
    return this;
  }

  /**
   * @protected
   * Flush data in internal buffer
   */  
  flush () {
    if (!this.writableFinished) {
      if (this._wbuf.length > 0) {
        this._write(this._wbuf, (err) => {
          if (err) {
            this.emit('error', err);
          } else {
            if (this._wbuf.length > 0) {
              setTimeout(() => this.flush(), 0);
            } else {
              this.emit('drain');
              this.finish();
            }
          }
        })
        this._wbuf = '';
      }
    }
  }
  
  /**
   * @protected
   * Finish to write on the stream
   */
  finish () {
    if (this.writableEnded && this._wbuf.length === 0) {
      this._final((err) => {
        if (err) {
          this.emit('error', err);
        } else {
          this._afterFinish();
        }
      });
    }
  }
}

/**
 * Duplex class
 */
class Duplex extends Writable /*, Readable */ {
  constructor () {
    super();
    this.readableEnded = false;
  }
  
  /**
   * @protected
   * Should be called when there is no more data to read.
   */
  _afterEnd () {
    if (!this.readableEnded) {
      this.readableEnded = true;
      this.emit('end');
    }
  }

  /**
   * @protected
   * Push a chunk of data to this readable stream.
   * @param {string} chunk
   * @return {this}
   */  
  push (chunk) {
    this.emit('data', chunk);
    return this;
  }  
}

exports.Readable = Readable;
exports.Writable = Writable;
exports.Duplex = Duplex;
