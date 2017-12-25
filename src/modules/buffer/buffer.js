var _builtin = process.binding(process.binding.buffer);

/*
function Buffer() {
}

Buffer.alloc = function (size) {
  var buffer = new Buffer();
  buffer._native = _builtin.alloc(size, 0);
  buffer.length = size;
  return buffer;
}

Buffer.from = function () {
  // ...
}

Buffer.isBuffer = function () {
  // ...
}

Buffer.prototype.[]
*/

module.exports = _builtin;
