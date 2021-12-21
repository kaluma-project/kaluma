const { test, start, expect } = require("__ujest");

test("[process] process.arch", (done) => {
  expect(process.arch).toBe('x86');
  done();
});

test("[process] process.platform", (done) => {
  expect(process.platform).toBe('linux');
  done();
});

test("[process] process.version", (done) => {
  expect(typeof process.version).toBe('string');
  done();
});

test("[process] process.memoryUsage()", (done) => {
  const mem = process.memoryUsage();
  expect(mem).toBeTruthy();
  expect(mem.heapTotal > mem.heapUsed).toBe(true);
  expect(mem.heapTotal > mem.heapPeak).toBe(true);
  expect(mem.heapPeak > mem.heapUsed).toBe(true);
  done();
});

test("[process] process.binding", (done) => {
  const natives = Object.keys(process.binding);
  const modules = process.builtin_modules;
  natives.forEach(m => {
    expect(modules).toContain(m);
  })
  done();
});

test("[process] process.stdin", (done) => {
  const stdin = process.stdin;
  expect(stdin).toBeTruthy();
  done();
});

test("[process] process.stdout", (done) => {
  const stdout = process.stdout;
  expect(stdout).toBeTruthy();
  done();
});

start(); // start to test
