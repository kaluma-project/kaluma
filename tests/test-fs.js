const { test, start, expect } = require("__ujest");
const { VFSLittleFS } = require("vfs_lfs");
const { RAMBlockDev } = require("__test_utils");
const fs = require("fs");
fs.register('lfs', VFSLittleFS);

test("[fs] mkfs()", (done) => {
  const bd1 = new RAMBlockDev();

  expect(() => {
    fs.mount('/', bd1, 'lfs');
  }).toThrow()

  fs.mkfs(bd1, 'lfs');
  fs.mount('/', bd1, 'lfs');

  expect(fs.__lookup("/")).toBeTruthy();

  fs.unmount('/');
  done();
});

test("[fs] mount() - with mkfs", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  expect(fs.__lookup("/")).toBeTruthy();

  fs.unmount('/');
  done();
});

test("[fs] mount() - multiple blkdev", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mkfs(bd1, 'lfs');
  fs.mount('/', bd1, 'lfs');

  const bd2 = new RAMBlockDev();
  fs.mkfs(bd2, 'lfs');
  fs.mount('/flash', bd2, 'lfs');

  const bd3 = new RAMBlockDev();
  fs.mount('/sd', bd3, 'lfs', true);

  expect(fs.__lookup("/")).toBeTruthy();
  expect(fs.__lookup("/flash")).toBeTruthy();
  expect(fs.__lookup("/sd")).toBeTruthy();

  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] unmount()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);
  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');

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
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);

  let r = null;

  r = fs.__lookup("/");
  expect(r.path).toBe("/");
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/flash");
  expect(r.path).toBe("/flash");
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/sd");
  expect(r.path).toBe("/sd");
  expect(r.__pathout).toBe("/");

  r = fs.__lookup("/flash/dir");
  expect(r.path).toBe("/flash");
  expect(r.__pathout).toBe("/dir");

  r = fs.__lookup("/flash1");
  expect(r.path).toBe("/");
  expect(r.__pathout).toBe("/flash1");

  r = fs.__lookup("/sd/2");
  expect(r.path).toBe("/sd");
  expect(r.__pathout).toBe("/2");

  r = fs.__lookup("/flash/1");
  expect(r.path).toBe("/flash");
  expect(r.__pathout).toBe("/1");

  r = fs.__lookup("/sd/1/dir");
  expect(r.path).toBe("/sd");
  expect(r.__pathout).toBe("/1/dir");

  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] cwd()", (done) => {
  expect(fs.cwd()).toBe("/");
  done();
});

test("[fs] chdir()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);

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

  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] readdirSync()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);  

  const bd4 = new RAMBlockDev();
  fs.mount('/sd/dev1', bd4, 'lfs', true);

  let ls = [];

  ls = fs.readdirSync("/");
  expect(ls).toContain("flash");
  expect(ls).toContain("sd");

  ls = fs.readdirSync("/sd");
  expect(ls).toContain("dev1");

  fs.unmount("/sd/dev1");
  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] mkdirSync() and rmdirSync()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

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

  fs.unmount('/');
  done();
});

test("[fs] open/write/read/close/unlink/statSync()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

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

  fs.unmount('/');
  done();
});

test("[fs] existsSync()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);  

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

  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] renameSync()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);  

  let fd = fs.openSync('/rename.txt', 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeSync(fd, buf, 0, buf.length, 0);
  fs.closeSync(fd);
  expect(fs.existsSync('/rename.txt')).toBe(true);

  fs.renameSync('rename.txt', 'newname.txt');
  expect(fs.existsSync('/rename.txt')).toBe(false);
  expect(fs.existsSync('/newname.txt')).toBe(true);

  fs.unlinkSync('newname.txt');

  fs.unmount('/');
  done();
});

test("[fs] write/readFileSync()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  const buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeFileSync('/filesync.txt', buf);
  expect(fs.existsSync('/filesync.txt')).toBe(true);

  const buf2 = fs.readFileSync('/filesync.txt');
  expect(buf.join(',')).toBe(buf2.join(','));

  fs.unlinkSync('/filesync.txt');

  fs.unmount('/');
  done();
});

// TODO: test for offset, length, position
// TODO: test for flags (wx, w+, r+, rs+, a, ax, a+, as, as+, ...)
// TODO: test for exceptions (e.g. try to read for non-exists file)
// TODO: createReadStream
// TODO: createWriteStream

start();
