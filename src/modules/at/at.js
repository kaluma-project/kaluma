var EventEmitter = require('events').EventEmitter;

// TODO: If "+IPD:" all response not arrived, skip all handlers.

/**
 * ATCommand class.
 * This class allows to handle AT command and the responses very easier.
 * This processes AT commands sequentially. It means that send AT command,
 * wait for the response, and then send the next AT command.
 * 
 * @param {UART} serial
 * @param {object} options
 *   .debug {boolean}
 *   .responseParser {function(at,data)}
 */
class ATCommand extends EventEmitter {
  constructor (serial, options) {
    super();
    this.job = null;
    this.queue = [];
    this.serial = serial;
    this.data = '';
    this.lineHandlers = {};
    this.responseHandlers = [];
    if (options) {
      this.debug = options.debug;
    }
    this.handler = (data) => {
      var s = String.fromCharCode.apply(null, new Uint8Array(data));
      if (this.debug) {
        print(s);
      }
      this.data += s;
      this.processResponseHandlers();
      this.processLineHandlers();
      // process jobs in queue
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
    while (ep < this.data.length) {
      if (this.data[ep] === '\n') {
        var l = this.data.substr(sp, ep - sp + 1).trim();
        var handled = handler(l);
        if (handled) {
          this.data = this.data.substr(0, sp) + this.data.substr(ep + 1);
          ep = sp;
        }
        sp = ep + 1;
        ep = sp;
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
    if (this.data.startsWith(line + '\r\n')) {
      return line.length + 2;
    } else {
      var idx = this.data.indexOf('\r\n' + line + '\r\n');
      if (idx > -1) {
        return idx + line.length + 4;
      }
    }
    return 0;
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
   *     data after elimination of handled data.
   * @param {function(data:string)} handler
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
      this.data = fn(this.data);
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
    })    
  }

  /**
   * Create a job to send AT command
   *
   * @param {string} cmd  AT command
   * @param {function(err:Error,res:any)} cb  Response callback
   * @param {Array<string>|function|number} match  A function to parse data from serial.
   *     It returns falsy value if response is not completed,
   *     Otherwise, it returns response data or Error object
   * @param {object} options
   *   - timeout {number} Set timeout. Default: 10000. (10s).
   *   - prepend {boolean} Add the job to first. Default: false.
   *   - clean {boolean} Clean the response data before sending AT command.
   */
  send(cmd, cb, match, options) {
    if (!options) options = {};
    var job = {
      cmd: cmd,
      cb: cb,
      match: match || ['OK', 'ERROR', 'FAIL'],
      running: false,
      timeout: options.timeout || 10000,
      prepend: options.prepend ? true : false,
      clean: options.clean ? true : false
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
          this.data = ''
        }
        this.serial.write(this.job.cmd + '\r\n');
        this.job.running = true;
        this.job.timer = setTimeout(() => {
          if (this.job && this.job.running) {
            if (this.job.cb) this.job.cb(null, 'TIMEOUT');
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
              res = this.data.substr(0, idx);
              result = match[i];
              this.data = this.data.substr(idx);
              break;
            }
          }
          if (result) {
            this.completeJob(this.job, res, result);
            this.next();
          }
        } else if (typeof match === 'number') {
          if (!this.job.waiting && match > 0) {
            if (this.job.timer) {
              clearTimeout(this.job.timer);
            }
            this.job.waiting = setTimeout(() => {
              var res = this.data;
              this.data = '';
              this.completeJob(this.job, res);
              this.next();
            }, match);
          }
        } else if (typeof match === 'function') {
          idx = match(this.data);
          if (idx > 0) {
            res = this.data.substr(0, idx);
            this.data = this.data.substr(idx);
            this.completeJob(this.job, res);
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
   * @param {string} response
   * @param {string} result
   */
  completeJob (job, response, result) {
    if (job.timer) {
      clearTimeout(job.timer);
    }
    if (job.cb) job.cb(response, result);
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
