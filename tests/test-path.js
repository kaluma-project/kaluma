const { test, start, expect } = require("__test");
const path = require("path");

test("[path] format() - 'root' ignored if 'dir' provided", (done) => {
  const pathObj = {
    root: "/ignored",
    dir: "/home/user/dir",
    base: "file.txt",
  };
  expect(path.format(pathObj)).toBe("/home/user/dir/file.txt");
  done(); // should be called
});

test("[path] format() - 'name', 'ext' ignored if 'base' provided", (done) => {
  const pathObj = {
    root: "/ignored",
    dir: "/home/user/dir",
    base: "file.txt",
    name: "ignored",
    ext: ".ignored",
  };
  expect(path.format(pathObj)).toBe("/home/user/dir/file.txt");
  done(); // should be called
});

test("[path] format() - 'root' is used if 'dir' not provided", (done) => {
  const pathObj = {
    root: "/",
    base: "file.txt",
    name: "ignored",
    ext: ".ignored",
  };
  expect(path.format(pathObj)).toBe("/file.txt");
  done(); // should be called
});

test("[path] format() - 'name', 'ext' are used if 'base' not provided", (done) => {
  const pathObj = {
    dir: "/home/user/dir",
    name: "file",
    ext: ".txt",
  };
  expect(path.format(pathObj)).toBe("/home/user/dir/file.txt");
  done(); // should be called
});

start(); // start to test
