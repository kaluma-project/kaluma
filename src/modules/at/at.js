var EventEmitter = require('events').EventEmitter;

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
  constructor (serial, options = {}) {
    super();
    this.job = null;
    this.queue = [];
    this.serial = serial;
    this.buffer = '';
    this.handlers = {};
    this.debug = options.debug;
    this.handler = (data) => {
      var s = String.fromCharCode.apply(null, data);
      if (this.debug) {
        print(s);
      }
      this.buffer += s;
      setTimeout(() => { this.process(); }, 0);
    };
    this.serial.on('data', this.handler);
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
   * Add a handler
   * @param {string} match
   * @param {function(line:string, buffer:string)} handler
   */
  addHandler(match, handler) {
    this.handlers[match] = handler;
  }
  
  /**
   * Remove a handler
   * @param {string} match
   */
  removeHandler(match) {
    delete this.handlers[match];
  }

  /**
   * Pop the first line in the buffer
   */
  pop() {
    var idx = this.buffer.indexOf('\n');
    if (idx > -1) {
      var l = this.buffer.substr(0, idx + 1);
      this.buffer = this.buffer.substr(idx + 1);
      return l;
    } else {
      return null;
    }
  }
  
  /**
   * Unpop a line
   */
  unpop(line) {
    this.buffer = line + this.buffer;
  }

  /**
   * Create a job to send AT command or data
   * @param {string|Uint8Array} cmd  AT command or data
   * @param {function(result:string)} cb  Response callback
   * @param {number|Array<string>} waitFor
   * @param {object} options
   *   - timeout {number} Set timeout. Default: 10000. (10s).
   *   - prepend {boolean} Add the job to first. Default: false.
   *   - sendAsData {boolean} Send cmd argument as data (without appending '\r\n')
   */
  send(cmd, cb, waitFor, options = {}) {
    var job = {
      cmd: cmd,
      cb: cb,
      result: null, // null | 1 = WAITEND | 2 = TIMEOUT | 3 = COMPLETE | "one of waitFor string"
      waitFor: waitFor || ['OK', 'ERROR', 'FAIL'],
      timeout: options.timeout || 10000,
      prepend: options.prepend ? true : false,
      sendAsData: options.sendAsData ? true : false
    };
    if (job.prepend) {
      this.queue.unshift(job);
    } else {
      this.queue.push(job);
    }
    setTimeout(() => { this.process(); }, 0);
  }

  /**
   * Pop a line and process handles. Repeat until meet the `until` param or no more lines.
   * @param {string} until
   * Process handlers 
   */
  processHandlers (until) {
    var line = this.pop();
    while (typeof line === 'string') {
      if (line.trim() === until) {
        return;
      } else {
        for (var match in this.handlers) {
          if (line.startsWith(match)) {
            var fn = this.handlers[match];
            var r = fn(line, this.buffer);
            if (r === false) {
              this.unpop(line);
              return;
            } else if (typeof r === 'string') {
              this.buffer = r;
              // if this buffer is modified,
              // that means more data is required to process
              // so immediately finish processing
              return;
            }
          }
        }
      }
      line = this.pop();
    }
  }

  /**
   * Process current job
   */
  processJob () {
    var waitFor = this.job.waitFor;
    if (!this.job.cb) {
      this.job.result = 1; // WAITEND
    }
    if (Array.isArray(waitFor)) {
      for (var i = 0; i < waitFor.length; i++) {
        if (this.hasLine(waitFor[i])) {
          this.job.result = waitFor[i];
          break;
        }
      }
    }
  }

  /**
   * process jobs and handlers
   */
  process() {
    if (!this.job) {
      this.startJob();
    }
    if (this.job) {
      this.processJob();
      if (this.job.result) {
        this.completeJob(this.job.result);
      }
    } else {
      this.processHandlers();
    }
  }

  /**
   * Start a job from the queue
   */
  startJob () {
    this.job = this.queue.shift();
    if (this.job) {
      var cmd = this.job.cmd;
      if (typeof cmd === 'string' && !this.job.sendAsData) cmd += '\r\n';
      this.serial.write(cmd);
      // set waiting timer
      if (typeof this.job.waitFor === 'number') {
        setTimeout(() => {
          if (this.job && !this.job.result) {
            this.job.result = 1; // WAITEND
            setTimeout(() => { this.process(); }, 0);
          }
        }, this.job.waitFor);
      }
      // set timeout timer
      if (this.job.timeout > 0) {
        setTimeout(() => {
          if (this.job && !this.job.result) {
            this.job.result = 2; // TIMEOUT
            setTimeout(() => { this.process(); }, 0);
          }
        }, this.job.timeout);
      }
    }
  }

  /**
   * Complete the job
   * @param {string} result
   */
  completeJob (result) {
    this.processHandlers(result); // process handlers until result
    if (result === 1) { // WAITEND
      if (this.job.cb) this.job.cb();
    } else if (result === 2) { // TIMEOUT
      if (this.job.cb) this.job.cb('TIMEOUT');
    } else if (typeof result === 'string' & result.length > 0) { // string match
      if (this.job.cb) this.job.cb(result);
    }
    this.job.result = 3;
    this.job = null;
    setTimeout(() => { this.process(); }, 0);
  }
}

exports.ATCommand = ATCommand;
