const { test, start, expect } = require("__ujest");
const { Flash } = require("flash");

const BLOCK_BASE = 0;
const BLOCK_COUNT = 260;
const BLOCK_SIZE = 4096;

test("[flash] constructor", (done) => {
  const flash = new Flash(BLOCK_BASE, BLOCK_COUNT);
  expect(flash.base).toBe(BLOCK_BASE);
  expect(flash.count).toBe(BLOCK_COUNT);
  done();
});

test("[flash] write() and read()", (done) => {
  const flash = new Flash(BLOCK_BASE, BLOCK_COUNT);

  // write to block[0].
  let buf1 = new Uint8Array(BLOCK_SIZE);
  buf1.fill(77);
  flash.write(0, buf1);

  // read from block[0]
  let buf2 = new Uint8Array(BLOCK_SIZE);
  buf2.fill(0);
  flash.read(0, buf2);

  // two buffers should be same
  expect(buf1.join(",")).toBe(buf2.join(","));
  done();
});

test("[flash] ioctl(4) - block count", (done) => {
  const flash = new Flash(BLOCK_BASE, BLOCK_COUNT);
  expect(flash.ioctl(4)).toBe(BLOCK_COUNT);
  done();
});

test("[flash] ioctl(5) - block size", (done) => {
  const flash = new Flash(BLOCK_BASE, BLOCK_COUNT);
  expect(flash.ioctl(5)).toBe(BLOCK_SIZE);
  done();
});

test("[flash] ioctl(6) - erase block", (done) => {
  const flash = new Flash(BLOCK_BASE, BLOCK_COUNT);

  // write to block[0].
  let buf1 = new Uint8Array(BLOCK_SIZE);
  buf1.fill(77);
  flash.write(0, buf1);

  // erase block[0] (filled with 255)
  flash.ioctl(6, 0);

  // read from block[0]
  let buf2 = new Uint8Array(BLOCK_SIZE);
  buf2.fill(0);
  flash.read(0, buf2);

  expect(buf2.reduce((a, b) => a + b)).toBe(BLOCK_SIZE * 255);
  done();
});

start(); // start to test
