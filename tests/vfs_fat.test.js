const { test, start, expect } = require("__ujest");
const { RAMBlockDev } = require("__test_utils");
const { VFSFatFS } = require("vfs_fat");

const bd = new RAMBlockDev(512, 1024, 512);
const vfs = new VFSFatFS(bd);

test("[vfs_fat] mount without format", (done) => {
  expect(() => {
    vfs.mount();
  }).toThrow();
  done();
});

test("[vfs_fat] mkfs and mount()", (done) => {
  vfs.mkfs();
  expect(() => {
    vfs.mount();
  }).notToThrow();
  done();
});

test("[vfs_fat] unmount()", (done) => {
  expect(() => {
    vfs.unmount();
  }).notToThrow();
  done();
});

test("[vfs_fat] mount()", (done) => {
  expect(() => {
    vfs.mount();
  }).notToThrow();
  done();
});

test("[vfs_fat] mkdir()", (done) => {
  vfs.mkdir("dir1");
  let files = vfs.readdir("/");
  expect(files).toContain("DIR1");
  done();
});

test("[vfs_fat] rmdir()", (done) => {
  let files = vfs.readdir("/");
  expect(files).toContain("DIR1");
  vfs.rmdir("dir1");
  files = vfs.readdir("/");
  expect(files).notToContain("DIR1");
  done();
});

test("[vfs_fat] readdir()", (done) => {
  vfs.mkdir("dir1");
  vfs.mkdir("dir2");
  vfs.mkdir("dir3");
  let files = vfs.readdir("/");
  expect(files.length).toBe(3);
  expect(files).toContain("DIR1");
  expect(files).toContain("DIR2");
  expect(files).toContain("DIR3");
  done();
});

test("[vfs_fat] rename()", (done) => {
  vfs.rename("dir3", "dir4");
  let files = vfs.readdir("/");
  expect(files).notToContain("DIR3");
  expect(files).toContain("DIR4");
  done();
});

test("[vfs_fat] open() / close()", (done) => {
  let id = vfs.open("/test.txt", 0x0A, 0);
  vfs.close(id);
  let files = vfs.readdir("/");
  expect(files).toContain("TEST.TXT");
  done();
});

test("[vfs_fat] stat()", (done) => {
  let stat_file = vfs.stat("/test.txt");
  expect(stat_file.type).toBe(1);
  expect(stat_file.size).toBe(0);
  let stat_dir = vfs.stat("/DIR4");
  expect(stat_dir.type).toBe(2);
  expect(stat_dir.size).toBe(0);
  done();
});

test("[vfs_fat] unlink", (done) => {
  vfs.unlink("/test.txt");
  let files = vfs.readdir("/");
  expect(files).notToContain("TEST.TXT");
  done();
});

test("[vfs_fat] write()", (done) => {
  let id = vfs.open("/test.txt", 0x0A, 0);
  let w_buffer = new Uint8Array([0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39]);
  vfs.write(id, w_buffer, 0, w_buffer.length, 0);
  vfs.close(id);
  let stat_file = vfs.stat("/test.txt");
  expect(stat_file.type).toBe(1);
  expect(stat_file.size).toBe(w_buffer.length);
  let files = vfs.readdir("/");
  expect(files).toContain("TEST.TXT");
  done();
});

test("[vfs_fat] read()", (done) => {
  let id = vfs.open("/test.txt", 0x01, 0);
  let w_buffer = new Uint8Array([0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39]);
  let r_buffer = new Uint8Array(10);
  vfs.read(id, r_buffer, 0, r_buffer.length, 0);
  vfs.close(id);
  expect(r_buffer.join(',')).toBe(w_buffer.join(','));
  done();
});

start();
