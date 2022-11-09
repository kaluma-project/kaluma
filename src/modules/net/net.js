const net_native = process.binding(process.binding.net);
const { Stream } = require("stream");
console.log("net!");

class Socket extends Stream {
  constructor() {
    super();
    this._native = new net_native.SocketNative();
  }

  /**
   * Initiates a connection
   * @param {{host:string, port:number}} options
   * @param {function} connectListener
   * @return {Socket}
   */
  connect(options, connectListener) {
    if (connectListener) {
      this.on("connect", connectListener);
    }
  }
}

exports.Socket = Socket;
