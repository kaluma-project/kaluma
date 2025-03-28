var EventEmitter = require('events').EventEmitter;

class WiFi extends EventEmitter {
  constructor() {
    super();
    if (!global.__ieee80211dev) {
      throw new Error('IEEE 802.11 device not found');
    }
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
  reset(cb) {
    if (this._dev) {
      this._dev.reset((err) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb(null);
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }

  /**
   * Scan networks
   * @param {Function} cb
   */
  scan(cb) {
    if (this._dev) {
      this._dev.scan((err, scanResults) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb(null, scanResults);
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }

  /**
   * Connect to network
   * @param {object} connectInfo
   *   .ssid {string}
   *   .password {string}
   * @param {function} cb
   */
  connect(connectInfo, cb) {
    if (this._dev) {
      if (typeof connectInfo === 'function') {
        cb = connectInfo
        connectInfo = null
      }
      if (!connectInfo) {
        connectInfo = { enforce: false }
      }
      if (!connectInfo.ssid) {
        connectInfo.ssid = storage ? storage.getItem('WIFI_SSID') : null
        if (!connectInfo.ssid) {
          if (cb) cb(new Error('WIFI SSID is required'));
          return;
        }
      }
      if (!connectInfo.password) {
        connectInfo.password = storage ? storage.getItem('WIFI_PASSWORD') : null
      }
      if (!connectInfo.security) {
        connectInfo.security = storage ? storage.getItem('WIFI_SECURITY') : null
      }
      if (connectInfo.enforce) {
        this._dev.connect(connectInfo, err => {
          if (err) {
            if (cb) cb(new SystemError(this._dev.errno));
          } else {
            if (cb) cb(null, connectInfo);
          }
        });
      } else {
        this._dev.get_connection((err, connectionInfo) => {
          if (err) {
            if (cb) cb(new SystemError(this._dev.errno));
          } else {
            if (!connectionInfo) {
              this._dev.connect(connectInfo, err => {
                if (err) {
                  if (cb) cb(new SystemError(this._dev.errno));
                } else {
                  if (cb) cb(null, connectInfo);
                }
              });
            } else {
              if (cb) cb(null, connectionInfo);
            }
          }
        });
      }
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }

  /**
   * Disconnect the current network connection
   * @param {function} cb
   */
  disconnect(cb) {
    if (this._dev) {
      this._dev.disconnect(err => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb();
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO      
    }
  }

  /**
   * Get the current network connection
   * @param {function(err,connectionInfo)} cb
   *   connectionInfo = null, if has no connection
   */
  getConnection(cb) {
    if (this._dev) {
      this._dev.get_connection((err, connectionInfo) => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb(null, connectionInfo);
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO      
    }
  }

  /**
   * Wifi AP mode (Optional driver function)
   * @param {object} apmodeInfo
   *   .ssid {string}
   *   .password {string}
   * @param {function} cb
   */
  wifiApMode(apmodeInfo, cb) {
    console.log(typeof this._dev.ap_mode)
    if ((this._dev) && (typeof this._dev.ap_mode === 'function')) {
      if (typeof apmodeInfo === 'function') {
        cb = apmodeInfo
        apmodeInfo = null
      }
      if (!apmodeInfo) {
        apmodeInfo = {ssid: "Kaluma_AP",
                      password: "kalumaAp",
                      gateway: "192.168.4.1",
                      subnet_mask: "255.255.255.0"}
      }
      if (!apmodeInfo.ssid) {
        apmodeInfo.ssid = storage ? storage.getItem('WIFI_AP_SSID') : "Kaluma_AP"
      }
      if (!apmodeInfo.password) {
        apmodeInfo.password = storage ? storage.getItem('WIFI_AP_PASSWORD') : "kalumaAp"
      }
      if (!apmodeInfo.gateway) {
        apmodeInfo.gateway = storage ? storage.getItem('WIFI_AP_GATEWAY') : "192.168.4.1"
      }
      if (!apmodeInfo.subnet_mask) {
        apmodeInfo.subnet_mask = storage ? storage.getItem('WIFI_AP_SUBNET_MASK') : "255.255.255.0"
      }
      this._dev.ap_mode(apmodeInfo, err => {
        if (err) {
          if (cb) cb(new SystemError(this._dev.errno));
        } else {
          if (cb) cb(null, apmodeInfo);
        }
      });
    } else {
      if (cb) cb(new SystemError(6)); // ENXIO
    }
  }

  /**
   * Disable Wifi AP mode (Optional driver function)
   */
  disableWifiApMode() {
    if ((this._dev) && (typeof this._dev.disable_ap_mode === 'function')) {
      return this._dev.disable_ap_mode();
    }
  }
  /**
   * Return Wifi AP clients (Optional driver function)
   */
  getWifiApClients() {
    if ((this._dev) && (typeof this._dev.get_ap_client === 'function')) {
      return this._dev.get_ap_client();
    }
  }
}

// var wifi = new WiFi();
// module.exports = wifi;
exports.WiFi = WiFi;
