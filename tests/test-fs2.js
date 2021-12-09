const { test, start, expect } = require("__ujest");
const { VFSLittleFS } = require("vfs_lfs");

const fs = require("fs");

const bd1 = new global.FlashBD(0, 16);
const bd2 = new global.FlashBD(16, 16);
const bd3 = new global.FlashBD(32, 16);
const vfs1 = new VFSLittleFS(bd1);
const vfs2 = new VFSLittleFS(bd2);
const vfs3 = new VFSLittleFS(bd3);

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
  const _vfs = new VFSLittleFS(new global.FlashBD(48, 16));
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

test("[fs] open/write/read/close/unlink/statSync()", (done) => {
  init_fs();
  const fname = '/file.txt';

  // file write (create)
  let fd = fs.openSync(fname, 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeSync(fd, buf, 0, buf.length, 0);
  fs.closeSync(fd);

  // file stat test
  let stat = fs.statSync(fname);
  expect(stat.isFile()).toBe(true);
  expect(stat.size).toBe(buf.length);

  // file read test
  let fd2 = fs.openSync(fname, 'r');
  let buf2 = new Uint8Array(10);
  fs.readSync(fd2, buf2, 0, buf2.length, 0);
  fs.closeSync(fd2);
  expect(buf.join(',')).toBe(buf2.join(','));

  fs.unlinkSync(fname);
  deinit_fs();
  done();
});

test("[fs] existsSync()", (done) => {
  init_fs();
  const fname = '/exists.txt';

  let fd = fs.openSync(fname, 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeSync(fd, buf, 0, buf.length, 0);
  fs.closeSync(fd);  
  expect(fs.existsSync(fname)).toBe(true);

  expect(fs.existsSync('/')).toBe(true);
  expect(fs.existsSync('/flash')).toBe(true);
  expect(fs.existsSync('/sd')).toBe(true);
  expect(fs.existsSync('/flash1')).toBe(false);

  fs.unlinkSync(fname);
  deinit_fs();
  done();
});

test("[fs] renameSync()", (done) => {
  init_fs();

  let fd = fs.openSync('/rename.txt', 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeSync(fd, buf, 0, buf.length, 0);
  fs.closeSync(fd);
  expect(fs.existsSync('/rename.txt')).toBe(true);

  fs.renameSync('rename.txt', 'newname.txt');
  expect(fs.existsSync('/rename.txt')).toBe(false);
  expect(fs.existsSync('/newname.txt')).toBe(true);

  fs.unlinkSync('newname.txt');
  deinit_fs();
  done();
});

test("[fs] write/readFileSync()", (done) => {
  init_fs();

  const buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeFileSync('/filesync.txt', buf);
  expect(fs.existsSync('/filesync.txt')).toBe(true);

  const buf2 = fs.readFileSync('/filesync.txt');
  expect(buf.join(',')).toBe(buf2.join(','));

  fs.unlinkSync('/filesync.txt');
  deinit_fs();
  done();
});

// TODO: test for offset, length, position
// TODO: test for flags (wx, w+, r+, rs+, a, ax, a+, as, as+, ...)
// TODO: test for exceptions (e.g. try to read for non-exists file)
// TODO: createReadStream
// TODO: createWriteStream

start();
