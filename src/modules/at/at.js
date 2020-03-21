var EventEmitter = require('events').EventEmitter;

// TODO: How to skip all handlers if all response not arrived of "+IPD:" message.

/**
 * ATCommand class.
 * This class allows to handle AT command and the responses very easier.
 * This processes AT commands sequentially. It means that send AT command,
 * wait for the response, and then send the next AT command.
 * 
 * @param {UART} serial
 * @param {object} options
 *   .debug {boolean}
 */
class ATCommand extends EventEmitter {
  constructor (serial, options) {
    super();
    this.job = null;
    this.queue = [];
    this.serial = serial;
    this.buffer = '';
    this.lineHandlers = {};
    this.responseHandlers = [];
    this.paused = false;
    if (options) {
      this.debug = options.debug;
    }
    this.handler = (data) => {
      var s = String.fromCharCode.apply(null, data);
      if (this.debug) {
        print(s);
      }
      this.buffer += s;
      if (!this.paused) {
        this.processResponseHandlers();
        this.processLineHandlers();
      }
      this.processJobs();
    }
    this.serial.on('data', this.handler);
  }

  /**
   * Efficiently iterate every lines with handler
   */
  iterateLines (handler) {
    var sp = 0;
    var ep = 0;
    while (ep < this.buffer.length) {
      if (this.buffer[ep] === '\n') {
        var l = this.buffer.substr(sp, ep - sp + 1).trim();
        var handled = handler(l);
        if (handled) {
          this.buffer = this.buffer.substr(0, sp) + this.buffer.substr(ep + 1);
          ep = sp;
        } else {
          sp = ep + 1;
          ep = sp;
        }
      } else {
        ep++;
      }
    }    
  }

  /**
   * @param {string} line
   * @return {number} Position of the end of matched line + 1.
   */
  hasLine(line) {
    if (this.buffer.startsWith(line + '\r\n')) {
      return line.length + 2;
    } else {
      var idx = this.buffer.indexOf('\r\n' + line + '\r\n');
      if (idx > -1) {
        return idx + line.length + 4;
      }
    }
    return 0;
  }

  /**
   * Pause to run process handlers
   */
  pause () {
    this.paused = true;
  }
  
  /**
   * Resume to run process handlers
   */
  resume () {
    this.paused = false;
    this.processResponseHandlers();
    this.processLineHandlers();
    this.buffer = '';
  }
  
  /**
   * Add a line handler
   * @param {string} match
   * @param {function(line:string)} handler
   */
  addLineHandler(match, handler) {
    this.lineHandlers[match] = handler
  }
  
  /**
   * Remove a line handler
   * @param {string} match
   */
  removeLineHandler(match) {
    delete this.lineHandlers[match];
  }

  /**
   * Add a response handler. A Reponse handler should return the rest of 
   *     response buffer after elimination of handled data.
   * @param {function(buffer:string)} handler
   */
  addResponseHandler(handler) {
    this.responseHandlers.push(handler)
  }
  
  /**
   * Remove a response handler
   * @param {function} handler
   */
  removeResponseHandler(handler) {
    var idx = this.responseHandlers.indexOf(handler)
    if (idx > -1) {
      this.responseHandlers.splice(idx, 1);
    }
  }

  /**
   * Process response handlers sequentially
   */
  processResponseHandlers() {
    this.responseHandlers.forEach(fn => {
      this.buffer = fn(this.buffer);
    })
  }

  /**
   * Process line handlers for a given line
   * @param {string} line
   */
  processLineHandlers () {
    this.iterateLines(line => {
      for (var match in this.lineHandlers) {
        if (line.startsWith(match)) {
          var fn = this.lineHandlers[match]
          fn(match, line);
          return true;
        }
      }
      return false;      
    });
  }

  /**
   * Create a job to send AT command or data
   * @param {string|Uint8Array} cmd  AT command or data
   * @param {function(result:string, response:string)} cb  Response callback
   * @param {Array<string>|function|number} match
   * @param {object} options
   *   - timeout {number} Set timeout. Default: 20000. (20s).
   *   - prepend {boolean} Add the job to first. Default: false.
   *   - clean {boolean} Clean the response data before sending AT command.
   *   - sendAsData {boolean} Send cmd argument as data (without appending '\r\n')
   */
  send(cmd, cb, match, options) {
    if (!options) options = {};
    var job = {
      cmd: cmd,
      cb: cb,
      match: match || ['OK', 'ERROR', 'FAIL'],
      running: false,
      timeout: options.timeout || 20000,
      prepend: options.prepend ? true : false,
      clean: options.clean ? true : false,
      sendAsData: options.sendAsData ? true : false
    }
    if (job.prepend) {
      this.queue.unshift(job);
    } else {
      this.queue.push(job);
    }
    this.processJobs();
  }
  
  /**
   * Process current job and wait for response
   */
  processJobs () {
    // start a job
    if (!this.job) {
      this.job = this.queue.shift();
      if (this.job) {
        if (this.job.clean) {
          this.buffer = ''
        }
        var cmd = this.job.cmd;
        if (typeof cmd === 'string' && !this.job.sendAsData) cmd += '\r\n';
        this.serial.write(cmd);
        this.job.running = true;
        this.job.timer = setTimeout(() => {
          if (this.job && this.job.running) {
            if (this.job.cb) this.job.cb('TIMEOUT');
            this.next();
          }
        }, this.job.timeout);
      }
    }

    // check whether job is complete or not
    if (this.job) {
      if (this.job.match) {
        var match = this.job.match;
        var res = null;
        if (Array.isArray(match)) {
          var result = null;
          for (var i = 0; i < match.length; i++) {
            var idx = this.hasLine(match[i]);
            if (idx) {
              res = this.buffer.substr(0, idx);
              result = match[i];
              if (!this.paused) {
                this.buffer = this.buffer.substr(idx);
              }
              break;
            }
          }
          if (result) {
            this.completeJob(this.job, result, res);
            this.next();
          }
        } else if (typeof match === 'number') {
          if (!this.job.waiting && match > 0) {
            if (this.job.timer) {
              clearTimeout(this.job.timer);
            }
            this.job.waiting = setTimeout(() => {
              var res = this.buffer;
              if (!this.paused) {
                this.buffer = '';
              }
              this.completeJob(this.job, null, res);
              this.next();
            }, match);
          }
        } else if (typeof match === 'function') {
          idx = match(this.buffer);
          if (idx > 0) {
            res = this.buffer.substr(0, idx);
            if (!this.paused) {
              this.buffer = this.buffer.substr(idx);
            }
            this.completeJob(this.job, null, res);
            this.next();
          }
        }
      } else { // if no matches, just go next job.
        this.next();
      }
    }
  }

  /**
   * Complete the job
   *
   * @param {object} job
   * @param {string} result
   * @param {string} response
   */
  completeJob (job, result, response) {
    if (job.timer) {
      clearTimeout(job.timer);
    }
    if (job.cb) job.cb(result, response);
    job.running = false;
  }
  
  /**
   * Process next job
   */
  next() {
    this.job = null;
    setTimeout(() => {
      this.processJobs()
    }, 10)
  }  
}

exports.ATCommand = ATCommand;
