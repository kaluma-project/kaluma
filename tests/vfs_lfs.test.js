const { test, start, expect } = require("__ujest");
const { RAMBlockDev } = require("__test_utils");
const { VFSLittleFS } = require("vfs_lfs");

// constants for flags
const VFS_FLAG_READ = 1;
const VFS_FLAG_WRITE = 2;
const VFS_FLAG_CREATE = 4;
const VFS_FLAG_APPEND = 8;
const VFS_FLAG_EXCL = 16;
const VFS_FLAG_TRUNC = 32;

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
  vfs.mkfs();
  expect(() => {
    vfs.mount();
  }).notToThrow();
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

test("[vfs_lfs] mount() - not formatted", (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  expect(() => {
    vfs.mount();
  }).toThrow();
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
  vfs.mkdir("dir2");
  vfs.mkdir("dir3");
  let files = vfs.readdir("/");
  expect(files.length).toBe(3);
  expect(files).toContain("dir1");
  expect(files).toContain("dir2");
  expect(files).toContain("dir3");
  vfs.unmount();
  done();
});

test("[vfs_lfs] mkdir() - already exists", (done) => {
  const vfs = init_vfs();
  vfs.mkdir("dir1");
  let files = vfs.readdir("/");
  expect(files.length).toBe(1);
  expect(files).toContain("dir1");

  // mkdir "dir1" already exists
  expect(() => {
    vfs.mkdir("dir1");
  }).toThrow("File exists");

  vfs.unmount();
  done();
});

test("[vfs_lfs] rmdir()", (done) => {
  const vfs = init_vfs();

  vfs.mkdir("dir1");
  vfs.mkdir("dir2");
  vfs.mkdir("dir3");
  let files = vfs.readdir("/");
  expect(files.length).toBe(3);
  expect(files).toContain("dir1");
  expect(files).toContain("dir2");
  expect(files).toContain("dir3");

  // rmdir dir1
  vfs.rmdir("dir1");
  files = vfs.readdir("/");
  expect(files.length).toBe(2);
  expect(files).notToContain("dir1");
  expect(files).toContain("dir2");
  expect(files).toContain("dir3");

  // rmdir dir2
  vfs.rmdir("dir2");
  files = vfs.readdir("/");
  expect(files.length).toBe(1);
  expect(files).notToContain("dir1");
  expect(files).notToContain("dir2");
  expect(files).toContain("dir3");

  // rmdir dir3
  vfs.rmdir("dir3");
  files = vfs.readdir("/");
  expect(files.length).toBe(0);
  expect(files).notToContain("dir1");
  expect(files).notToContain("dir2");
  expect(files).notToContain("dir3");

  vfs.unmount();
  done();
});

test("[vfs_lfs] rmdir() - not exists", (done) => {
  const vfs = init_vfs();

  // rmdir dir1 not exists
  expect(() => {
    vfs.rmdir("dir1");
  }).toThrow("No such file or directory");

  vfs.unmount();
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

  vfs.unmount();
  done();
});

test("[vfs_lfs] readdir() - not exists", (done) => {
  const vfs = init_vfs();

  expect(() => {
    vfs.readdir("/dir");
  }).toThrow("No such file or directory");

  vfs.unmount();
  done();
});

test("[vfs_lfs] open/write/read/close/unlink/stat()", (done) => {
  const vfs = init_vfs();
  const fname = "/file.txt";

  // file write (create)
  let fd = vfs.open(fname, VFS_FLAG_WRITE | VFS_FLAG_CREATE, 0);
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  vfs.write(fd, buf, 0, buf.length, 0);
  vfs.close(fd);

  // file stat
  let stat = vfs.stat(fname);
  expect(stat.type).toBe(1); // 1=file, 2=dir
  expect(stat.size).toBe(buf.length);

  // file read
  let fd2 = vfs.open(fname, VFS_FLAG_READ, 0);
  let buf2 = new Uint8Array(10);
  vfs.read(fd2, buf2, 0, buf2.length, 0);
  vfs.close(fd2);
  expect(buf.join(",")).toBe(buf2.join(","));

  // file unlink
  vfs.unlink(fname);
  expect(() => {
    let fd3 = vfs.open(fname, VFS_FLAG_READ, 0);
  }).toThrow("No such file or directory");

  vfs.unmount();
  done();
});

test("[vfs_lfs] open() - for read not exists", (done) => {
  const vfs = init_vfs();
  const fname = "/nofile.txt";
  expect(() => {
    vfs.open(fname, VFS_FLAG_READ, 0);
  }).toThrow("No such file or directory");
  vfs.unmount();
  done();
});

test("[vfs_lfs] open() - for write with VFS_FLAG_EXCL to already exists", (done) => {
  const vfs = init_vfs();
  const fname = "/file.txt";

  // file write (create)
  let fd = vfs.open(fname, VFS_FLAG_WRITE | VFS_FLAG_CREATE, 0);
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  vfs.write(fd, buf, 0, buf.length, 0);
  vfs.close(fd);

  // write exclusive on already exists
  expect(() => {
    vfs.open(fname, VFS_FLAG_WRITE | VFS_FLAG_EXCL, 0);
  }).toThrow("File exists");

  vfs.unmount();
  done();
});

test("[vfs_lfs] write(fd, buffer) - without offset, length, position", (done) => {
  const vfs = init_vfs();
  const fname = "/written.txt";

  // file write sequentially
  let fd = vfs.open(fname, VFS_FLAG_WRITE | VFS_FLAG_CREATE, 0);
  let wbuf1 = new Uint8Array([60, 61, 62, 63, 64]);
  let wbuf2 = new Uint8Array([65, 66, 67, 68, 69]);
  let wbuf3 = new Uint8Array([70, 71, 72, 73, 74]);
  vfs.write(fd, wbuf1);
  vfs.write(fd, wbuf2);
  vfs.write(fd, wbuf3);
  vfs.close(fd);

  // file size
  let stat = vfs.stat(fname);
  expect(stat.type).toBe(1); // 1=file,2=dir
  expect(stat.size).toBe(wbuf1.length + wbuf2.length + wbuf3.length);

  // file read sequentially
  let fd2 = vfs.open(fname, VFS_FLAG_READ, 0);
  let rbuf1 = new Uint8Array(wbuf1.length);
  let rbuf2 = new Uint8Array(wbuf2.length);
  let rbuf3 = new Uint8Array(wbuf3.length);
  vfs.read(fd2, rbuf1);
  vfs.read(fd2, rbuf2);
  vfs.read(fd2, rbuf3);
  vfs.close(fd2);
  expect(wbuf1.join(",")).toBe(rbuf1.join(","));
  expect(wbuf2.join(",")).toBe(rbuf2.join(","));
  expect(wbuf3.join(",")).toBe(rbuf3.join(","));

  // remove file
  vfs.unlink(fname);

  // unmount
  vfs.unmount();
  done();
});

test("[vfs_lfs] rename()", (done) => {
  const vfs = init_vfs();

  // create file "rename.txt"
  let fd = vfs.open("rename.txt", VFS_FLAG_WRITE | VFS_FLAG_CREATE, 0);
  let buf = new Uint8Array([60, 61, 62, 63, 64, 65, 66, 67, 68, 69]);
  vfs.write(fd, buf, 0, buf.length, 0);
  vfs.close(fd);

  // check file exists
  let stat1 = vfs.stat("rename.txt");
  expect(stat1.type).toBe(1); // 1=file,2=dir
  expect(stat1.size).toBe(buf.length);

  // rename "rename.txt" to "newname.txt"
  vfs.rename("rename.txt", "newname.txt");

  // "rename.txt" not exists
  expect(() => {
    vfs.stat("rename.txt");
  }).toThrow("No such file or directory");

  // "newname.txt" exists
  let stat2 = vfs.stat("newname.txt");
  expect(stat2.type).toBe(1); // 1=file,2=dir
  expect(stat2.size).toBe(buf.length);

  vfs.unlink("newname.txt");
  vfs.unmount();
  done();
});

start();
