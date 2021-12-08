const { test, start, expect } = require("__ujest");
const { RAMBlockDev } = require("__test_utils");
const { VFSLittleFS } = require("vfs_lfs");

function init_vfs() {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mount();
  return vfs;
}

test("[vfs_lfs] mount()", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  expect(() => {
    vfs.mount();
  }).notToThrow();
  done();
});

test("[vfs_lfs] unmount()", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
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

/*
interface VFS {
  constructor(blockdev)
  mount()
  unmount()
  open(path: string, flags: number, mode: number): number (id)
  write(id: number, buffer: Uint8Array, offset: number, length: number, position: number): number (bytes written)
  read(id: number, buffer: Uint8Array, offset: number, length: number, position: number): number (bytes read)
  close(id: number)
  stat(path: string) -> {type:number (1=file,2=dir), size:number}
  mkdir(path: string)
  rmdir(path: string)
  readdir(path: string) -> string[]
  rename(oldPath: string, newPath: string)
  unlink(path: string)
}
*/

start();
