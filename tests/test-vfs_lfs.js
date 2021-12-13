const { test, start, expect } = require("__ujest");
const { RAMBlockDev } = require("__test_utils");
const { VFSLittleFS } = require("vfs_lfs");

function init_vfs() {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mkfs();
  vfs.mount();
  return vfs;
}

test("[vfs_lfs] mkfs()", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  expect(() => {
    vfs.mount();
  }).toThrow();
  done();
});

test("[vfs_lfs] mount()", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mkfs();
  expect(() => {
    vfs.mount();
  }).notToThrow();
  done();
});

test("[vfs_lfs] unmount()", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mkfs();
  vfs.mount();
  expect(() => {
    vfs.unmount();
  }).notToThrow();
  done();
});

test("[vfs_lfs] mkdir()", (done) => {
  const vfs = init_vfs();
  vfs.mkdir("dir1");
  let files = vfs.readdir("/");
  expect(files).toContain("dir1");
  done();
});

test("[vfs_lfs] rmdir()", (done) => {
  const vfs = init_vfs();
  vfs.mkdir("dir1");
  let files = vfs.readdir("/");
  expect(files).toContain("dir1");
  vfs.rmdir("dir1");
  files = vfs.readdir("/");
  expect(files).notToContain("dir1");
  done();
});

test("[vfs_lfs] readdir()", (done) => {
  const vfs = init_vfs();
  vfs.mkdir("dir1");
  vfs.mkdir("dir2");
  vfs.mkdir("dir3");
  let files = vfs.readdir("/");
  expect(files.length).toBe(3);
  expect(files).toContain("dir1");
  expect(files).toContain("dir2");
  expect(files).toContain("dir3");
  done();
});

start();
