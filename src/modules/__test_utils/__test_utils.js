class RAMBlockDev {
  constructor(size = 4096, count = 16, bufsz = 256) {
    this.blocksize = size;
    this.blockcount = count;
    this.buffersize = bufsz;
    this.buf = new Uint8Array(this.blocksize * this.blockcount);
    this.buf.fill(255);
  }

  read(block, buffer, offset = 0) {
    // console.log(`bd::read ${block} ${buffer.length} ${offset}`);
    for (let i = 0; i < buffer.length; i++) {
      buffer[i] = this.buf[block * this.blocksize + offset + i];
    }
  }

  write(block, buffer, offset = 0) {
    // console.log(`bd::write ${block} ${buffer.length} ${offset}`);
    for (let i = 0; i < buffer.length; i++) {
      this.buf[block * this.blocksize + offset + i] = buffer[i];
    }
  }

  ioctl(op, arg) {
    // console.log(`bd:ioctl ${op} ${arg}`);
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
          this.buf[i] = 255;
        }
        return 0;
      case 7: // buffer size
        return this.buffersize;
    }
    return -1; // unknown op
  }
}

class RAMFatBlkDev {
  constructor() {
    this.blocksize = 512;
    this.blockcount = 1024;
    this.buffersize = 512;
    this.buf = new Uint8Array(this.blocksize * this.blockcount);
    this.buf.fill(255);
  }

  read(block, buffer, offset = 0) {
    // console.log(`bd::read ${block} ${buffer.length} ${offset}`);
    for (let i = 0; i < buffer.length; i++) {
      buffer[i] = this.buf[block * this.blocksize + offset + i];
    }
  }

  write(block, buffer, offset = 0) {
    // console.log(`bd::write ${block} ${buffer.length} ${offset}`);
    for (let i = 0; i < buffer.length; i++) {
      this.buf[block * this.blocksize + offset + i] = buffer[i];
    }
  }

  ioctl(op, arg) {
    // console.log(`bd:ioctl ${op} ${arg}`);
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
          this.buf[i] = 255;
        }
        return 0;
      case 7: // buffer size
        return this.buffersize;
    }
    return -1; // unknown op
  }
}

exports.RAMBlockDev = RAMBlockDev;
exports.RAMFatBlkDev = RAMFatBlkDev;
