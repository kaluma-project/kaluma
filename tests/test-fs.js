const {test, start, expect} = require('__test');
const fs = require('fs');
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

function init_fs() {
  const vfs = new VFSLittleFS(new RAMBlockDev());
  const vfs1 = new VFSLittleFS(new RAMBlockDev());
  const vfs2 = new VFSLittleFS(new RAMBlockDev());
  fs.mount('/', vfs);
  fs.mount('/flash', vfs1);
  fs.mount('/sd', vfs2);
}

test('[fs] cwd()', (done) => {
  expect(fs.cwd()).toBe('/');
  done();
});

test('[fs] chdir()', (done) => {
  expect(() => { fs.chdir('/unknown') }).toThrow();
  done();
});

test('[fs] readdirSync()', (done) => {
  init_fs();
  let files = fs.readdirSync('/');
  // expect(files).toContain('/flash');
  // expect(files).toContain('/sd');
  done();
});

start();
