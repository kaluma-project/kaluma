const { test, start, expect } = require("__ujest");
const { Socket } = require("net");

const socket = new Socket();
console.log(socket);

/*
test("[net] socket connect", (done) => {
  var event_conn = 0;
  var event_ready = 0;
  var conn = { host: "192.168.0.11", port: 3000 };
  var sock = net.createConnection(conn, () => {
    event_conn++;
    expect(event_conn).toBe(1);
    // TODO: sock.localAddress
    // TODO: sock.localPort
    // TODO: sock.remoteAddress
    // TODO: sock.remotePort
  });
  sock.on("ready", () => {
    event_ready++;
    // ready is occurred immediately after connect
    expect(event_conn).toBe(1);
    expect(event_ready).toBe(1);
    done();
  });
  sock.on("error", (err2) => {
    done(err2);
  });
});
*/

// start(); // start to test

console.log(1, 2, 3);
