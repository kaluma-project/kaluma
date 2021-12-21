const { test, start, expect } = require("__ujest");
const { Stream } = require("stream");

test("[stream] Stream initial values", (done) => {
  const stream = new Stream();
  expect(stream.destroyed).toBe(false);
  expect(stream.readable).toBe(false);
  expect(stream.readableFlowing).toBe(false);
  expect(stream.readableEnded).toBe(false);
  expect(stream.writable).toBe(false);
  expect(stream.writableEnded).toBe(false);
  expect(stream.writableFinished).toBe(false);
  done();
});

start(); // start to test
