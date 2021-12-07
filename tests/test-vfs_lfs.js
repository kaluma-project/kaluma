const {test, start, expect} = require('__test');
const {VFSLittleFS} = require('vfs_lfs');

class RAMBlockDev {
  constructor () {
    this.blocksize = 4096;
    this.blockcount = 16;
    this.buf = new Uint8Array(this.blocksize * this.blockcount);    
    this.buf.fill(0);
  }

  read (block, buffer, offset = 0) {
    for (let i = 0; i < buffer.length; i++) {
      buffer[i] = this.buf[block * this.blocksize + offset + i];
    }
  }
  
  write (block, buffer, offset = 0) {
    for (let i = 0; i < buffer.length; i++) {
      this.buf[block * this.blocksize + offset + i] = buffer[i];
    }    
  }
  
  ioctl (op, arg) {
    switch (op) {
      case 1: // init
        return 0;
      case 2: // shutdown
        return 0;
      case 3: // sync
        return 0;
      case 4: // block count
        return this.blockcount;
      case 5: // block size
        return this.blocksize;
      case 6: // erase block
        let p = arg * this.blocksize;
        for (let i = p; i < p + this.blocksize; i++) {
          this.buf[i] = 0;
        }
        return 0;
    }
    return -1; // unknown op
  }
}

function init_vfs() {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mount();
  return vfs;
}

test('[vfs_lfs] mount()', (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  expect(() => { vfs.mount() }).notToThrow();
  done();
});

test('[vfs_lfs] unmount()', (done) => {
  const bd = new RAMBlockDev();
  const vfs = new VFSLittleFS(bd);
  vfs.mount();
  expect(() => { vfs.unmount() }).notToThrow();
  done();
});

test('[vfs_lfs] mkdir()', (done) => {
  const vfs = init_vfs();
  vfs.mkdir('dir1');
  let files = vfs.readdir('/');
  expect(files).toContain('dir1');
  done();
});

test('[vfs_lfs] rmdir()', (done) => {
  const vfs = init_vfs();
  vfs.mkdir('dir1');
  let files = vfs.readdir('/');
  expect(files).toContain('dir1');
  vfs.rmdir('dir1');
  files = vfs.readdir('/');
  expect(files).notToContain('dir1');
  done();
});

test('[vfs_lfs] readdir()', (done) => {
  const vfs = init_vfs();
  vfs.mkdir('dir1');
  vfs.mkdir('dir2');
  vfs.mkdir('dir3');
  let files = vfs.readdir('/');
  expect(files.length).toBe(3);
  expect(files).toContain('dir1');
  expect(files).toContain('dir2');
  expect(files).toContain('dir3');
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
