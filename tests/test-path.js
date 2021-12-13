const { test, start, expect } = require("__ujest");
const { VFSLittleFS } = require("vfs_lfs");
const { RAMBlockDev } = require("__test_utils");
const path = require("path");
const fs = require("fs");
fs.register("lfs", VFSLittleFS);

test("[path] sep === '/'", (done) => {
  expect(path.sep).toBe("/");
  done();
});

test("[path] delimiter === ':'", (done) => {
  expect(path.sep).toBe("/");
  done();
});

test("[path] format() - 'root' ignored if 'dir' provided", (done) => {
  const pathObj = {
    root: "/ignored",
    dir: "/home/user/dir",
    base: "file.txt",
  };
  expect(path.format(pathObj)).toBe("/home/user/dir/file.txt");
  done();
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
  done();
});

test("[path] format() - 'root' is used if 'dir' not provided", (done) => {
  const pathObj = {
    root: "/",
    base: "file.txt",
    name: "ignored",
    ext: ".ignored",
  };
  expect(path.format(pathObj)).toBe("/file.txt");
  done();
});

test("[path] format() - 'name', 'ext' are used if 'base' not provided", (done) => {
  const pathObj = {
    dir: "/home/user/dir",
    name: "file",
    ext: ".txt",
  };
  expect(path.format(pathObj)).toBe("/home/user/dir/file.txt");
  done();
});

test("[path] isAbsolute()", (done) => {
  const _path1 = "/home";
  const _path2 = "../usr/include";
  expect(path.isAbsolute(_path1)).toBe(true);
  expect(path.isAbsolute(_path2)).toBe(false);
  done();
});

test("[path] parse()", (done) => {
  const _path = "/home/user/dir/file.ext";
  const _pathObj = path.parse(_path);
  expect(_pathObj.root).toBe("/");
  expect(_pathObj.dir).toBe("home/user/dir");
  expect(_pathObj.base).toBe("file.ext");
  expect(_pathObj.name).toBe("file");
  expect(_pathObj.ext).toBe(".ext");
  done();
});

test("[path] dirname()", (done) => {
  expect(path.dirname("/home/user/dir/file.ext")).toBe("/home/user/dir");
  expect(path.dirname("home/user/dir/file")).toBe("home/user/dir");
  done();
});

test("[path] extname()", (done) => {
  expect(path.extname("/home/user/dir/file.ext")).toBe(".ext");
  expect(path.extname("/home/user/dir/file.bmp.json")).toBe(".json");
  done();
});

test("[path] extname()", (done) => {
  expect(path.extname("/home/user/dir/file.ext")).toBe(".ext");
  expect(path.extname("/home/user/dir/file.bmp.json")).toBe(".json");
  done();
});

test("[path] basename()", (done) => {
  expect(path.basename("/home/user/dir/file.ext")).toBe("file.ext");
  expect(path.basename("/home/user/dir/file.ext", ".ext")).toBe("file");
  done();
});

test("[path] normalize()", (done) => {
  expect(path.normalize("/home/user/../file.ext")).toBe("/home/file.ext");
  expect(path.normalize("/home/user/./file.ext")).toBe("/home/user/file.ext");
  expect(path.normalize("/home////user//file.ext")).toBe("/home/user/file.ext");
  expect(path.normalize("/")).toBe("/");
  done();
});

test("[path] join()", (done) => {
  expect(path.join("/home/user/", "../file.ext")).toBe("/home/file.ext");
  expect(path.join("/home/user/", "./file.ext")).toBe("/home/user/file.ext");
  expect(path.join("/home/user", "/file.ext")).toBe("/home/user/file.ext");
  expect(path.join("/home/user/", "..")).toBe("/home");
  expect(path.join("/", "..")).toBe("/");
  expect(path.join("/", ".")).toBe("/");
  done();
});

test("[path] resolve()", (done) => {
  const bd = new RAMBlockDev();
  fs.mount("/", bd, "lfs", true);

  fs.chdir("/");
  expect(path.resolve("/")).toBe("/");
  expect(path.resolve("/foo/bar", "./baz")).toBe("/foo/bar/baz");
  expect(path.resolve("/foo/bar", "/tmp/file/")).toBe("/tmp/file");

  fs.mkdirSync("/home");
  fs.mkdirSync("/home/myself");
  fs.mkdirSync("/home/myself/node");
  fs.chdir("/home/myself/node");
  expect(path.resolve("wwwroot", "static_files/png/", "../gif/image.gif")).toBe(
    "/home/myself/node/wwwroot/static_files/gif/image.gif"
  );

  fs.unmount("/");
  done();
});

start(); // start to test
