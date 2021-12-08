const path_mod = require("path");

class Stats {
  constructor() {
    this.type = 0;
    this.size = 0;
    /*
    dev: 2114,
    ino: 48064969,
    mode: 33188,
    nlink: 1,
    uid: 85,
    gid: 100,
    rdev: 0,
    size: 527,
    blksize: 4096,
    blocks: 8,
    atimeMs: 1318289051000.1,
    mtimeMs: 1318289051000.1,
    ctimeMs: 1318289051000.1,
    birthtimeMs: 1318289051000.1,
    atime: Mon, 10 Oct 2011 23:24:11 GMT,
    mtime: Mon, 10 Oct 2011 23:24:11 GMT,
    ctime: Mon, 10 Oct 2011 23:24:11 GMT,
    birthtime: Mon, 10 Oct 2011 23:24:11 GMT
    */
  }
  isDirectory() {
    return this.type === 2;
  }
  isFile() {
    return this.type === 1;
  }
}
// class fs.ReadStream
// class fs.WriteStream

/*
interface BlockDev {
  read(bnum, buf, offset);
  write(bnum, buf, offset);
  ioctrl(op, arg);
   - 1: initialize the device
   - 2: shutdown the device
   - 3: sync the device
   - 4: get a count of the number of blocks
   - 5: get the number of bytes in a block
   - 6: erase a block (arg = block num)
}

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
(throws SystemError)
*/

// constants for flags
const VFS_FLAG_READ = 1;
const VFS_FLAG_WRITE = 2;
const VFS_FLAG_CREATE = 4;
const VFS_FLAG_APPEND = 8;
const VFS_FLAG_EXCL = 16;
const VFS_FLAG_TRUNC = 32;

/**
 * VFS mount table
 * @type {Array.<{path: string, vfs:VFS}>}
 */
const __vfs = [];

/**
 * file objects (array index is file descriptor)
 * @type {Array.<{id: number, vfs:VFS}>}
 */
const __files = [];
__files.push({ id: 0, vfs: null }); // fd = 0 (linux stdin)
__files.push({ id: 1, vfs: null }); // fd = 1 (linux stdout)
__files.push({ id: 2, vfs: null }); // fd = 2 (linux stderr)

/**
 * Current working directory
 * @type {string}
 */
let __cwd = "/";

/**
 * Get file descriptor of file object
 * @param {object} fo
 * @returns {number}
 */
function __fd(fo) {
  let fd = -1;
  for (let i = 0; i < __files.length; i++) {
    if (__files[i] === null) {
      fd = i;
      __files[fd] = fo;
      return fd;
    }
  }
  fd = __files.push(fo) - 1;
  return fd;
}

/**
 * Get file object from file descriptor
 * @param {number} fd
 * @returns {object}
 */
function __fobj(fd) {
  return fd < __files.length ? __files[fd] : null;
}

/**
 * Lookup VFS with pathout
 * @param {string} path
 * @returns {VFS}
 */
function __lookup(path) {
  const _path = path_mod.resolve(path);
  for (let i = 0; i < __vfs.length; i++) {
    let vfs = __vfs[i];
    if (vfs.path === "/") {
      vfs.__pathout = _path;
      return vfs;
    } else if (
      _path === vfs.path ||
      _path.startsWith(vfs.path + path_mod.sep)
    ) {
      vfs.__pathout = _path.substr(vfs.path.length) || "/";
      return vfs;
    }
  }
  throw new SystemError(-2);
}

/**
 * Mount a VFS with path
 * @param {string} path
 * @param {VFS} vfs
 */
function mount(path, vfs) {
  path = path_mod.normalize(path);
  const _parent = path_mod.join(path, "..");
  if (_parent !== "/") {
    const _stat = statSync(_parent);
    if (!_stat.isDirectory()) {
      throw new SystemError(-2);
    }
  }
  vfs.path = path;
  vfs.mount();
  __vfs.push(vfs);
  __vfs.sort((a, b) => {
    let ac = a.path.split(path_mod.sep).filter((t) => t.length > 0).length;
    let bc = b.path.split(path_mod.sep).filter((t) => t.length > 0).length;
    return bc - ac;
  });
}

/**
 * Unmount VFS
 * @param {string} path
 */
function unmount(path) {
  path = path_mod.normalize(path);
  const vfs = __vfs.find((v) => v.path === path);
  if (vfs) {
    __vfs.splice(__vfs.indexOf(vfs));
  }
}

/**
 * Return current working directory
 * @returns {string}
 */
function cwd() {
  return __cwd;
}

/**
 * Change current working directory
 * @param {string} path
 */
function chdir(path) {
  const _path = path_mod.resolve(path);
  const stat = statSync(_path);
  if (stat && stat.isDirectory()) {
    __cwd = _path;
  } else {
    throw new SystemError(-2);
  }
}

function createReadStream(path, options) {
  // return fs_native.createReadStream(path, options);
}

function createWriteStream(path, options) {
  // return fs_native.createWriteStream(path, options);
}

// ---------------------------------------------------------------------------
// SYNCHRONOUS FUNCTIONS
// ---------------------------------------------------------------------------

function closeSync(fd) {
  const fo = __fobj(fd);
  if (fo) {
    let ret = fo.vfs.close(fo.id);
    if (ret > -1) {
      files[fd] = null;
    }
  } else {
    throw new SystemError(-9);
  }
}

function existsSync(path) {
  const vfs = __lookup(path);
  if (vfs && vfs.stat(vfs.__pathout)) {
    return true;
  }
  return false;
}

function openSync(path, flags = "r", mode = 0o666) {
  const vfs = __lookup(path);
  if (vfs) {
    let vfs_flags = 0;
    switch (flags) {
      case "r":
      case "rs":
      case "sr":
        vfs_flags = VFS_FLAG_READ;
        break;
      case "r+":
      case "rs+":
      case "sr+":
        vfs_flags = VFS_FLAG_READ | VFS_FLAG_WRITE;
        break;
      case "w":
        vfs_flags = VFS_FLAG_WRITE;
        break;
      case "wx":
      case "xw":
        vfs_flags = VFS_FLAG_WRITE | VFS_FLAG_EXCL;
        break;
      case "w+":
        vfs_flags = VFS_FLAG_WRITE | VFS_FLAG_READ | VFS_FLAG_TRUNC;
        break;
      case "wx+":
      case "xw+":
        vfs_flags = VFS_FLAG_WRITE | VFS_FLAG_READ | VFS_FLAG_EXCL;
        break;
      case "a":
        vfs_flags = VFS_FLAG_APPEND;
        break;
      case "ax":
      case "xa":
        vfs_flags = VFS_FLAG_APPEND | VFS_FLAG_EXCL;
        break;
      case "a+":
        vfs_flags = VFS_FLAG_APPEND | VFS_FLAG_READ;
        break;
      case "ax+":
      case "xa+":
        vfs_flags = VFS_FLAG_APPEND | VFS_FLAG_READ | VFS_FLAG_EXCL;
        break;
    }
    let id = vfs.open(vfs.__pathout, vfs_flags, mode);
    // TODO: if id < 0, id is error code
    let fo = {
      id: id,
      vfs: vfs,
    };
    let fd = __fd(fo);
    return fd;
  }
  // TODO: if no vfs found, what error should be thrown?
  // file not found?
  return -1;
}

function readSync(fd, buffer, offset, length, position) {
  // return fs_native.readSync(fd, buffer, offset, length, position);
}

function readFileSync(path) {
  // return fs_native.readFileSync(path);
}

function renameSync(oldPath, newPath) {
  // return fs_native.renameSync(oldPath, newPath);
}

function mkdirSync(path) {
  const vfs = __lookup(path);
  vfs.mkdir(vfs.__pathout);
}

function rmdirSync(path) {
  const vfs = __lookup(path);
  vfs.rmdir(vfs.__pathout);
}

function readdirSync(path) {
  path = path_mod.normalize(path);
  const vfs = __lookup(path);
  let ls = vfs.readdir(vfs.__pathout);
  __vfs.forEach((v) => {
    if (v.path !== "/" && path_mod.join(v.path, "..") === path) {
      ls.push(v.path.substr(path === "/" ? 1 : path.length + 1));
    }
  });
  return ls;
}

function statSync(path) {
  const vfs = __lookup(path);
  const ret = vfs.stat(vfs.__pathout);
  let stat = new Stats();
  stat.type = ret.type;
  stat.size = ret.size;
  return stat;
}

function unlinkSync(path) {
  // return fs_native.unlinkSync(path);
}

function writeSync(fd, buffer, offset, length, position) {
  if (fd < files.length) {
    const fo = files[fd];
    return fo.vfs.write(fo.id, buffer, offset, length, position);
  }
  // if invalid fd is given, what error should be thrown?
}

function writeFileSync(path, data) {
  // return fs_native.writeFileSync(path, data);
}

// ---------------------------------------------------------------------------
// ASYNCHRONOUS CALLBACK FUNCTIONS
// ---------------------------------------------------------------------------

// function close(fd[, callback])
// function exists(path[, callback])
// function mkdir(path[, options], callback)
// function open(path[, flags[, mode]], callback)
// function read(fd, buffer, offset, length, position, callback)
// function readdir(path, callback)
// function readFile(path, callback)
// function rename(oldPath, newPath, callback)
// function rmdir(path, callback)
// function stat(path, callback)
// function unlink(path, callback)
// function write(fd, buffer[, offset[, length[, position]]], callback)
// function writeFile(path, data, callback)

exports.Stats = Stats;
// exports.ReadStream = ReadStream
// exports.WriteStream = WriteStream
exports.createReadStream = createReadStream;
exports.createWriteStream = createWriteStream;

// for debugging
exports.__vfs = __vfs;
exports.__files = __files;
exports.__lookup = __lookup;
exports.__fd = __fd;
exports.__fobj = __fobj;

exports.mount = mount;
exports.unmount = unmount;
exports.chdir = chdir;
exports.cwd = cwd;
exports.closeSync = closeSync;
exports.existsSync = existsSync;
exports.mkdirSync = mkdirSync;
exports.openSync = openSync;
exports.readSync = readSync;
exports.readdirSync = readdirSync;
exports.readFileSync = readFileSync;
exports.renameSync = renameSync;
exports.rmdirSync = rmdirSync;
exports.statSync = statSync;
exports.unlinkSync = unlinkSync;
exports.writeSync = writeSync;
exports.writeFileSync = writeFileSync;
