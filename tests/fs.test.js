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

test("[fs] mount() - already formatted", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mkdir('/dir1');
  fs.unmount('/');

  fs.mount('/', bd1, 'lfs');
  expect(fs.__lookup("/")).toBeTruthy();
  expect(fs.readdir('/')).toContain('dir1');

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

  fs.mkdir("/usr");
  fs.mkdir("/usr/dir");

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

test("[fs] readdir()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);  

  const bd4 = new RAMBlockDev();
  fs.mount('/sd/dev1', bd4, 'lfs', true);

  let ls = [];

  ls = fs.readdir("/");
  expect(ls).toContain("flash");
  expect(ls).toContain("sd");

  ls = fs.readdir("/sd");
  expect(ls).toContain("dev1");

  fs.unmount("/sd/dev1");
  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] mkdir() and rmdir()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  let ls = [];

  fs.mkdir("/home");
  ls = fs.readdir("/");
  expect(ls).toContain("home");

  fs.mkdir("/home/usr");
  ls = fs.readdir("/home");
  expect(ls).toContain("usr");

  expect(fs.stat("/home").isDirectory()).toBe(true);
  expect(fs.stat("/home").isFile()).toBe(false);
  expect(fs.stat("/home/usr").isDirectory()).toBe(true);
  expect(fs.stat("/home/usr").isFile()).toBe(false);

  fs.rmdir("/home/usr");
  ls = fs.readdir("/home");
  expect(ls).notToContain("usr");

  fs.rmdir("/home");
  ls = fs.readdir("/");
  expect(ls).notToContain("home");

  fs.unmount('/');
  done();
});

test("[fs] open/write/read/close/unlink/stat()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  const fname = '/file.txt';

  // file write (create)
  let fd = fs.open(fname, 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.write(fd, buf, 0, buf.length, 0);
  fs.close(fd);

  // file stat test
  let stat = fs.stat(fname);
  expect(stat.isFile()).toBe(true);
  expect(stat.size).toBe(buf.length);

  // file read test
  let fd2 = fs.open(fname, 'r');
  let buf2 = new Uint8Array(10);
  fs.read(fd2, buf2, 0, buf2.length, 0);
  fs.close(fd2);
  expect(buf.join(',')).toBe(buf2.join(','));

  fs.unlink(fname);

  fs.unmount('/');
  done();
});

test("[fs] write(fd, buffer) - without offset, length, position", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  const fname = '/written.txt';

  // file write sequentially
  let fd = fs.open(fname, 'w');
  let wbuf1 = new Uint8Array([60, 61, 62, 63, 64]);
  let wbuf2 = new Uint8Array([65, 66, 67, 68, 69]);
  let wbuf3 = new Uint8Array([70, 71, 72, 73, 74]);
  fs.write(fd, wbuf1);
  fs.write(fd, wbuf2);
  fs.write(fd, wbuf3);
  fs.close(fd);

  // file size
  let stat = fs.stat(fname);
  expect(stat.isFile()).toBe(true);
  expect(stat.size).toBe(wbuf1.length + wbuf2.length + wbuf3.length);

  // file read sequentially
  let fd2 = fs.open(fname, 'r');
  let rbuf1 = new Uint8Array(wbuf1.length);
  let rbuf2 = new Uint8Array(wbuf2.length);
  let rbuf3 = new Uint8Array(wbuf3.length);
  fs.read(fd2, rbuf1);
  fs.read(fd2, rbuf2);
  fs.read(fd2, rbuf3);
  fs.close(fd2);
  expect(wbuf1.join(',')).toBe(rbuf1.join(','));
  expect(wbuf2.join(',')).toBe(rbuf2.join(','));
  expect(wbuf3.join(',')).toBe(rbuf3.join(','));

  // remove file
  fs.unlink(fname);

  // unmount
  fs.unmount('/');
  done();
});


test("[fs] exists()", (done) => {
  const bd1 = new RAMBlockDev();
  const bd2 = new RAMBlockDev();
  const bd3 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);
  fs.mount('/flash', bd2, 'lfs', true);
  fs.mount('/sd', bd3, 'lfs', true);  

  const fname = '/exists.txt';

  let fd = fs.open(fname, 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.write(fd, buf, 0, buf.length, 0);
  fs.close(fd);  
  expect(fs.exists(fname)).toBe(true);

  expect(fs.exists('/')).toBe(true);
  expect(fs.exists('/flash')).toBe(true);
  expect(fs.exists('/sd')).toBe(true);
  expect(fs.exists('/flash1')).toBe(false);

  fs.unlink(fname);

  fs.unmount('/');
  fs.unmount('/flash');
  fs.unmount('/sd');
  done();
});

test("[fs] rename()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);  

  let fd = fs.open('/rename.txt', 'w');
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.write(fd, buf, 0, buf.length, 0);
  fs.close(fd);
  expect(fs.exists('/rename.txt')).toBe(true);

  fs.rename('rename.txt', 'newname.txt');
  expect(fs.exists('/rename.txt')).toBe(false);
  expect(fs.exists('/newname.txt')).toBe(true);

  fs.unlink('newname.txt');

  fs.unmount('/');
  done();
});

test("[fs] write/readFile()", (done) => {
  const bd1 = new RAMBlockDev();
  fs.mount('/', bd1, 'lfs', true);

  const buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  fs.writeFile('/filesync.txt', buf);
  expect(fs.exists('/filesync.txt')).toBe(true);

  const buf2 = fs.readFile('/filesync.txt');
  expect(buf.join(',')).toBe(buf2.join(','));

  fs.unlink('/filesync.txt');

  fs.unmount('/');
  done();
});

// TODO: test for offset, length, position
// TODO: test for flags (wx, w+, r+, rs+, a, ax, a+, as, as+, ...)
// TODO: test for exceptions (e.g. try to read for non-exists file)
// TODO: createReadStream
// TODO: createWriteStream

start();
