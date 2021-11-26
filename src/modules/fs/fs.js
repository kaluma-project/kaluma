const fs_native = process.binding(process.binding.fs);

// class fs.Stats
// class fs.ReadStream
// class fs.WriteStream

// function close(fd[, callback])
function closeSync(fd) {
  return fs_native.closeSync(fd);
}

function createReadStream(path, options) {
  return fs_native.createReadStream(path, options);
}

function createWriteStream(path, options) {
  return fs_native.createWriteStream(path, options);
}

// function exists(path[, callback])
function existsSync(path) {
  return fs_native.existsSync(path);
}

// function fstat(fd, callback)
function fstatSync(fd) {
  return fs_native.fstatSync(fd);
}

// function mkdir(path[, options], callback)
function mkdirSync(path, options) {
  options = Object.assign({ mode: 0o777 }, options);
  return fs_native.mkdirSync(path, options.mode);
}

// function open(path[, flags[, mode]], callback)
function openSync(path, flags = "r", mode = 0o666) {
  return fs_native.openSync(path, flags, mode);
}

// function read(fd, buffer, offset, length, position, callback)
function readSync(fd, buffer, offset, length, position) {
  return fs_native.readSync(fd, buffer, offset, length, position);
}

// function readdir(path, callback)
function readdirSync(path) {
  return fs_native.readdirSync(path);
}

// function readFile(path, callback)
function readFileSync(path) {
  return fs_native.readFileSync(path);
}

// function rename(oldPath, newPath, callback)
function renameSync(oldPath, newPath) {
  return fs_native.renameSync(oldPath, newPath);
}

// function rmdir(path, callback)
function rmdirSync(path) {
  return fs_native.rmdirSync(path);
}

// function stat(path, callback)
function statSync(path) {
  return fs_native.statSync(path);
}

// function unlink(path, callback)
function unlinkSync(path) {
  return fs_native.unlinkSync(path);
}

// function write(fd, buffer[, offset[, length[, position]]], callback)
function writeSync(fd, buffer, offset, length, position) {
  return fs_native.writeSync(fd, buffer, offset, length, position);
}

// function writeFile(path, data, callback)
function writeFileSync(path, data) {
  return fs_native.writeFileSync(path, data);
}

exports.closeSync = closeSync;
exports.createReadStream = createReadStream;
exports.createWriteStream = createWriteStream;
exports.existsSync = existsSync;
exports.fstatSync = fstatSync;
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
