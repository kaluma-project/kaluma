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
 *   .interval {number}
 */
class ATCommand extends EventEmitter {
  constructor (serial, options = {}) {
    super();
    this.queue = [];
    this.serial = serial;
    this.buffer = '';
    this.handlers = {};
    this.options = Object.assign({
      debug: false,
      interval: 100
    }, options);
    this.handler = (data) => {
      var s = String.fromCharCode.apply(null, data);
      if (this.options.debug) {
        print(`\x1b[37m${s.replace(/\r/gi, '<CR>').replace(/\n/gi, '<LN>\n')}\x1b[0m`); // gray color
      }
      this.buffer += s;
    };
    this.serial.on('data', this.handler);
    // start periodical processing
    this._timer = setInterval(() => {
      this.process();
    }, this.options.interval);
  }
  
  close () {
    this.serial.removeListener('data', this.handler);
    if (this._timer) {
      clearInterval(this._timer);
      this._timer = null;
    }
  }

  _log(msg) {
    if (this.options.debug) {
      console.log(`\x1b[33m[AT] ${msg}\x1b[0m`); // brown color
    }
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
      let l = this.buffer.substr(0, idx + 1);
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
   *   - sendAsData {boolean} Send cmd argument as data (without appending '\r\n')
   */
  send(cmd, cb, waitFor, options = {}) {
    var job = {
      cmd: cmd,
      cb: cb,
      time: 0,
      started: false,
      waitFor: waitFor || ['OK', 'ERROR', 'FAIL'],
      timeout: options.timeout || 10000,
      sendAsData: options.sendAsData ? true : false
    };
    this.queue.push(job);
    this._log(`job queued [cmd="${cmd}"]`);
  }
  
  _startJob() {
    // start a pending job if buffer is empty
    if (this.queue.length > 0) {
      let job = this.queue[0];
      if (!job.started) {
        let cmd = job.cmd;
        if (typeof cmd === 'string' && !job.sendAsData) cmd += '\r\n';
        this.serial.write(cmd);
        job.started = true;
        job.time = millis(); // started time
        this._log(`job started [cmd="${cmd}"]`);
      }
    }
  }

  /**
   * Complete the job
   * @param {string} result
   */
  _completeJob (job, result) {
    try {
      if (result === 'end') {
        if (job.cb) job.cb();
      } else if (result === 'timeout') {
        if (job.cb) job.cb('TIMEOUT');
      } else { // expect string match
        if (job.cb) job.cb(result);
      }
    } catch (err) {
      console.error(err);
    }
    this._log(`job complete [cmd="${job.cmd}", result=${result}]`);
    this.queue.shift();
  }

  process() {
    var line = this.pop();
    while (typeof line === 'string') {
      // process job's expected results
      if (this.queue.length > 0) {
        let job = this.queue[0];
        if (job.started) {
          if (Array.isArray(job.waitFor))  {
            var l = line.trim();
            if (job.waitFor.indexOf(l) > -1) {
              this._completeJob(job, l);
              return;
            }
          }
        }
      }
      // process handlers
      for (let match in this.handlers) {
        if (line.startsWith(match)) {
          let fn = this.handlers[match];
          let r = fn(line, this.buffer);
          if (r === false) {
            // more data required to process
            this.unpop(line);
            return;
          } else if (typeof r === 'string') {
            // handler processed
            this.buffer = r;
          }
          this._log(`handler processed [match="${match}"]`);
          return;
        }
      }
      line = this.pop();
    }
    // start a job if no more lines to process
    if (line === null) {
      this._startJob();
    }
    // check job's timeout
    if (this.queue.length > 0) {
      let job = this.queue[0];
      if (job.started) {
        let ct = millis(); // current time
        let et = Math.abs(ct - job.time); // elapsed time
        if (typeof job.waitFor === 'number') {
          if (et > job.waitFor) {
            this._completeJob(job, 'end');
            return;
          }
        }
        if (Array.isArray(job.waitFor))  {
          if (et > job.timeout) {
            this._completeJob(job, 'timeout');
            return;
          }
        }
      }
    }
  }
}

exports.ATCommand = ATCommand;
