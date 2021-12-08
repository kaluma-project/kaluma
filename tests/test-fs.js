const { test, start, expect } = require("__ujest");
const { VFSLittleFS } = require("vfs_lfs");
const { RAMBlockDev } = require("__test_utils");
const fs = require("fs");

const vfs1 = new VFSLittleFS(new RAMBlockDev());
const vfs2 = new VFSLittleFS(new RAMBlockDev());
const vfs3 = new VFSLittleFS(new RAMBlockDev());

function init_fs() {
  fs.__vfs = [];
  fs.mount("/", vfs1);
  fs.mount("/flash", vfs2);
  fs.mount("/sd", vfs3);
}

function deinit_fs() {
  fs.unmount("/");
  fs.unmount("/flash");
  fs.unmount("/sd");
}

test("[fs] mount()", (done) => {
  init_fs();
  expect(fs.__lookup("/")).toBeTruthy();
  expect(fs.__lookup("/flash")).toBeTruthy();
  expect(fs.__lookup("/sd")).toBeTruthy();
  deinit_fs();
  done();
});

test("[fs] unmount()", (done) => {
  init_fs();
  deinit_fs();
  expect(() => {
    fs.__lookup("/");
  }).toThrow();
  expect(() => {
    fs.__lookup("/flash");
  }).toThrow();
  expect(() => {
    fs.__lookup("/sd");
  }).toThrow();
  done();
});

test("[fs] __lookup()", (done) => {
  init_fs();
  let r = null;

  r = fs.__lookup("/");
  expect(r).toBe(vfs1);
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/flash");
  expect(r).toBe(vfs2);
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/sd");
  expect(r).toBe(vfs3);
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/flash/dir");
  expect(r).toBe(vfs2);
  expect(r.__pathout).toBe("/dir");

  r = fs.__lookup("/flash1");
  expect(r).toBe(vfs1);
  expect(r.__pathout).toBe("/flash1");

  r = fs.__lookup("/sd/2");
  expect(r).toBe(vfs3);
  expect(r.__pathout).toBe("/2");

  r = fs.__lookup("/flash/1");
  expect(r).toBe(vfs2);
  expect(r.__pathout).toBe("/1");

  r = fs.__lookup("/sd/1/dir");
  expect(r).toBe(vfs3);
  expect(r.__pathout).toBe("/1/dir");

  deinit_fs();
  done();
});

test("[fs] cwd()", (done) => {
  expect(fs.cwd()).toBe("/");
  done();
});

test("[fs] chdir()", (done) => {
  init_fs();

  fs.chdir("/");
  expect(fs.cwd()).toBe("/");

  fs.chdir("/flash");
  expect(fs.cwd()).toBe("/flash");

  fs.chdir("/sd");
  expect(fs.cwd()).toBe("/sd");

  expect(() => {
    fs.chdir("/unknown");
  }).toThrow();

  fs.mkdirSync("/usr");
  fs.mkdirSync("/usr/dir");

  fs.chdir("/");
  expect(fs.cwd()).toBe("/");
  fs.chdir("usr");
  expect(fs.cwd()).toBe("/usr");
  fs.chdir("dir");
  expect(fs.cwd()).toBe("/usr/dir");
  fs.chdir("..");
  expect(fs.cwd()).toBe("/usr");
  fs.chdir(".");
  expect(fs.cwd()).toBe("/usr");
  fs.chdir("..");
  expect(fs.cwd()).toBe("/");
  fs.chdir("..");
  expect(fs.cwd()).toBe("/");

  deinit_fs();
  done();
});

test("[fs] readdirSync()", (done) => {
  init_fs();
  const _vfs = new VFSLittleFS(new RAMBlockDev());
  fs.mount("/sd/dev1", _vfs);
  let ls = [];

  ls = fs.readdirSync("/");
  expect(ls).toContain("flash");
  expect(ls).toContain("sd");

  ls = fs.readdirSync("/sd");
  expect(ls).toContain("dev1");

  fs.unmount("/sd/dev1");
  deinit_fs();
  done();
});

test("[fs] mkdirSync() and rmdirSync()", (done) => {
  init_fs();
  let ls = [];

  fs.mkdirSync("/home");
  ls = fs.readdirSync("/");
  expect(ls).toContain("home");

  fs.mkdirSync("/home/usr");
  ls = fs.readdirSync("/home");
  expect(ls).toContain("usr");

  expect(fs.statSync("/home").isDirectory()).toBe(true);
  expect(fs.statSync("/home").isFile()).toBe(false);
  expect(fs.statSync("/home/usr").isDirectory()).toBe(true);
  expect(fs.statSync("/home/usr").isFile()).toBe(false);

  fs.rmdirSync("/home/usr");
  ls = fs.readdirSync("/home");
  expect(ls).notToContain("usr");

  fs.rmdirSync("/home");
  ls = fs.readdirSync("/");
  expect(ls).notToContain("home");

  deinit_fs();
  done();
});

// existsSync
// statSync
// openSync, readSync, writeSync, closeSync
// unlinkSync
// renameSync
// readFileSync
// writeFileSync
// createReadStream
// createWriteStream

start();
