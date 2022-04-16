process.binding(process.binding.fs); // init native fs
const __path = require("path");

class Stats {
  constructor() {
    this.type = 0;
    this.size = 0;
  }
  isDirectory() {
    return this.type === 2;
  }
  isFile() {
    return this.type === 1;
  }
}

// constants for flags
const VFS_FLAG_READ = 1;
const VFS_FLAG_WRITE = 2;
const VFS_FLAG_CREATE = 4;
const VFS_FLAG_APPEND = 8;
const VFS_FLAG_EXCL = 16;
const VFS_FLAG_TRUNC = 32;

/**
 * Filesystem types
 * @type {Object<string, constructor>}
 */
const __fs = {};

/**
 * mount table
 * @type {Array<VFS>}
 */
const __mounts = [];

/**
 * open files (array index is file descriptor)
 * @type {Array.<{id: number, vfs:VFS}>}
 */
const __opens = [];

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
  for (let i = 0; i < __opens.length; i++) {
    if (__opens[i] === null) {
      fd = i;
      __opens[fd] = fo;
      return fd;
    }
  }
  fd = __opens.push(fo) - 1;
  return fd;
}

/**
 * Get file object from file descriptor
 * @param {number} fd
 * @returns {object}
 */
function __fobj(fd) {
  return fd < __opens.length ? __opens[fd] : null;
}

/**
 * Lookup VFS with pathout
 * @param {string} path
 * @returns {object} mount table entry
 */
function __lookup(path) {
  const _path = __path.resolve(path);
  for (let i = 0; i < __mounts.length; i++) {
    let vfs = __mounts[i];
    if (vfs.path === "/") {
      vfs.__pathout = _path;
      return vfs;
    } else if (_path === vfs.path || _path.startsWith(vfs.path + __path.sep)) {
      vfs.__pathout = _path.substr(vfs.path.length) || "/";
      return vfs;
    }
  }
  throw new SystemError(-2); // ENOENT
}

/**
 * Register a filesystem
 * @param {string} fstype
 * @param {constructor} fsctr
 */
function register(fstype, fsctr) {
  __fs[fstype] = fsctr;
}

/**
 * Unregister a filesystem
 * @param {string} fstype
 */
function unregister(fstype) {
  delete __fs[fstype];
}

/**
 * Make filesystem
 * @param {BlockDevice} blkdev
 * @param {string} fstype
 */
function mkfs(blkdev, fstype) {
  // create vfs of fstype
  const fsctr = __fs[fstype];
  if (!fsctr) {
    throw new SystemError(-22); // EINVAL (?)
  }
  const vfs = new fsctr(blkdev);
  vfs.mkfs();
}

/**
 * Mount a VFS with path
 * @param {string} path
 * @param {BlockDevice} blkdev
 * @param {string} fstype
 * @param {boolean} mkfs
 */
function mount(path, blkdev, fstype, mkfs) {
  path = __path.normalize(path);
  const _parent = __path.join(path, "..");
  if (_parent !== "/") {
    const _stat = stat(_parent);
    if (!_stat.isDirectory()) {
      throw new SystemError(-2); // ENOENT
    }
  }

  // create vfs of fstype
  const fsctr = __fs[fstype];
  if (!fsctr) {
    throw new SystemError(-22); // EINVAL (?)
  }
  const vfs = new fsctr(blkdev);

  // try to mount (try mkfs if mount failed)
  try {
    vfs.mount();
  } catch (err) {
    if (mkfs === true) {
      vfs.mkfs();
      vfs.mount();
    } else {
      throw err;
    }
  }

  vfs.path = path;
  __mounts.push(vfs);
  __mounts.sort((a, b) => {
    let ac = a.path.split(__path.sep).filter((t) => t.length > 0).length;
    let bc = b.path.split(__path.sep).filter((t) => t.length > 0).length;
    return bc - ac;
  });
}

/**
 * Unmount VFS
 * @param {string} path
 */
function unmount(path) {
  path = __path.normalize(path);
  const vfs = __mounts.find((v) => v.path === path);
  if (vfs) {
    vfs.unmount();
    __mounts.splice(__mounts.indexOf(vfs), 1);
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
  const _path = __path.resolve(path);
  const _stat = stat(_path);
  if (_stat && _stat.isDirectory()) {
    __cwd = _path;
  } else {
    throw new SystemError(-2); // ENOENT
  }
}

// ---------------------------------------------------------------------------
// SYNCHRONOUS FUNCTIONS
// ---------------------------------------------------------------------------

function open(path, flags = "r", mode = 0o666) {
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
        vfs_flags = VFS_FLAG_WRITE | VFS_FLAG_CREATE | VFS_FLAG_TRUNC;
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
    let fo = {
      id: id,
      vfs: vfs,
    };
    let fd = __fd(fo);
    return fd;
  }
  throw new SystemError(-2); // ENOENT
}

function read(fd, ...args) {
  const fo = __fobj(fd);
  if (fo) {
    return fo.vfs.read(fo.id, ...args);
  }
  throw new SystemError(-9); // EBADF
}

function readFile(path) {
  const _stat = stat(path);
  const buffer = new Uint8Array(_stat.size);
  const fd = open(path, "r");
  read(fd, buffer, 0, buffer.length, 0);
  close(fd);
  return buffer;
}

function write(fd, ...args) {
  const fo = __fobj(fd);
  if (fo) {
    return fo.vfs.write(fo.id, ...args);
  }
  throw new SystemError(-9); // EBADF
}

function writeFile(path, data) {
  const fd = open(path, "w");
  write(fd, data, 0, data.length, 0);
  close(fd);
}

function close(fd) {
  const fo = __fobj(fd);
  if (fo) {
    fo.vfs.close(fo.id);
    __opens[fd] = null;
  } else {
    throw new SystemError(-9); // EBADF
  }
}

function unlink(path) {
  const vfs = __lookup(path);
  if (vfs) {
    vfs.unlink(vfs.__pathout);
  } else {
    throw new SystemError(-2); // ENOENT
  }
}

function rename(oldPath, newPath) {
  const vfs = __lookup(oldPath);
  if (vfs) {
    vfs.rename(oldPath, newPath);
  } else {
    throw new SystemError(-2); // ENOENT
  }
}

function exists(path) {
  const vfs = __lookup(path);
  if (vfs) {
    try {
      let _stat = vfs.stat(vfs.__pathout);
      return _stat ? true : false;
    } catch (err) {
      return false;
    }
  }
  return false;
}

function mkdir(path) {
  const vfs = __lookup(path);
  vfs.mkdir(vfs.__pathout);
}

function rmdir(path) {
  const vfs = __lookup(path);
  vfs.rmdir(vfs.__pathout);
}

function readdir(path) {
  path = __path.normalize(path);
  const vfs = __lookup(path);
  let ls = vfs.readdir(vfs.__pathout);
  __mounts.forEach((v) => {
    if (v.path !== "/" && __path.join(v.path, "..") === path) {
      ls.push(v.path.substr(path === "/" ? 1 : path.length + 1));
    }
  });
  return ls;
}

function stat(path) {
  const vfs = __lookup(path);
  const _stat = vfs.stat(vfs.__pathout);
  let statObj = new Stats();
  statObj.type = _stat.type;
  statObj.size = _stat.size;
  return statObj;
}

function rm(path) {
  const _stat = stat(path);
  if (_stat.isDirectory()) {
    rmdir(path);
  } else if (_stat.isFile()) {
    unlink(path);
  }
}

exports.Stats = Stats;

// for debugging
exports.__fs = __fs;
exports.__mounts = __mounts;
exports.__opens = __opens;
exports.__lookup = __lookup;
exports.__fd = __fd;
exports.__fobj = __fobj;

exports.register = register;
exports.unregister = unregister;
exports.mkfs = mkfs;
exports.mount = mount;
exports.unmount = unmount;
exports.chdir = chdir;
exports.cwd = cwd;
exports.close = close;
exports.exists = exists;
exports.mkdir = mkdir;
exports.open = open;
exports.read = read;
exports.readdir = readdir;
exports.readFile = readFile;
exports.rename = rename;
exports.rmdir = rmdir;
exports.stat = stat;
exports.unlink = unlink;
exports.write = write;
exports.writeFile = writeFile;
exports.rm = rm;
