const storage_native = process.binding(process.binding.storage);

function sweep() {
  let cache = {};
  let len = storage_native.length();
  for (let i = 0; i < len; i++) {
    let k1 = storage_native.key(i);
    cache[k1] = storage_native.getItem(k1);
  }
  storage_native.clear();
  let keys = Object.keys(cache);
  for (let j = 0; j < keys.length; j++) {
    let k2 = keys[j];
    storage_native.setItem(k2, cache[k2]);
  }
}

exports.setItem = function (key, value) {
  try {
    storage_native.setItem(key, value);
  } catch (err) {
    if (err.errno === -142) {
      // storage full
      sweep();
      storage_native.setItem(key, value);
    } else {
      throw err;
    }
  }
};

exports.getItem = function (key) {
  return storage_native.getItem(key);
};

exports.removeItem = function (key) {
  return storage_native.removeItem(key);
};

exports.clear = function () {
  return storage_native.clear();
};

exports.key = function (index) {
  return storage_native.key(index);
};

Object.defineProperty(exports, "length", {
  get: storage_native.length,
});
