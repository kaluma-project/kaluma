var EventEmitter = require('events').EventEmitter;

if (!global.__ieee80211dev) {
  throw new Error('IEEE 802.11 device not found');
}

class WiFi extends EventEmitter {
  constructor () {
    super();
    this._dev = global.__ieee80211dev;
    this._dev.assoc_cb = () => {
      this.emit('associated');
    };
    this._dev.connect_cb = () => {
      this.emit('connected');
    };
    this._dev.disconnect_cb = () => {
      this.emit('disconnected');
    };
  }

  /**
   * Reset device
   * @param {Function} cb
   */
  reset (cb) {
    this._dev.reset((err) => {
      if (err) {
        if (cb) cb(new SystemError(this._dev.errno));
      } else {
        if (cb) cb(null);
      }
    });
  }

  /**
   * Scan networks
   * @param {Function} cb
   */
  scan (cb) {
    this._dev.scan((err, scanResults) => {
      if (err) {
        if (cb) cb(new SystemError(this._dev.errno));
      } else {
        if (cb) cb(null, scanResults);
      }
    });
  }
  
  /**
   * Connect to network
   * @param {object} connectInfo
   *   .ssid {string}
   *   .password {string}
   * @param {function} cb
   */
  connect (connectInfo, cb) {
    this._dev.connect(connectInfo, err => {
      if (err) {
        if (cb) cb(new SystemError(this._dev.errno));
      } else {
        if (cb) cb();
      }
    });
  }

  /**
   * Disconnect the current network connection
   * @param {function} cb
   */
  disconnect (cb) {
    this._dev.disconnect(err => {
      if (err) {
        if (cb) cb(new SystemError(this._dev.errno));
      } else {
        if (cb) cb();
      }
    });
  }

  /**
   * Get the current network connection
   * @param {function(err,connectionInfo)} cb
   *   connectionInfo = null, if has no connection
   */
  getConnection (cb) {
    this._dev.get_connection((err, connectionInfo) => {
      if (err) {
        if (cb) cb(new SystemError(this._dev.errno));
      } else {
        if (cb) cb(null, connectionInfo);
      }
    });
  }
    
}

var wifi = new WiFi();
module.exports = wifi;
