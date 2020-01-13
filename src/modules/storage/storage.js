var storage_native = process.binding(process.binding.storage);

class Storage {
  setItem (key, value) {
    var res = storage_native.setItem(key, value.toString());
    if (res === -1) {
      return undefined;
    } else if (res === -2) { // sweep required
      var cache = {}
      var len = storage_native.length();
      for (var i = 0; i < len; i++) {
        var k1 = storage_native.key(i)
        cache[k1] = storage_native.getItem(k1);
      }
      storage_native.clear();
      var keys = Object.keys(cache);
      for (var j = 0; j < keys.length; j++) {
        var k2 = keys[j]
        storage_native.setItem(k2, cache[k2]);
      }
      res = storage_native.setItem(key, value.toString());  
    } else if (res === -3) { // storage full
      return new Error("Storage full");
    } else if (res === -4) { // over length
      return new Error("The length of key and value is too long");
    }
  }

  getItem (key) {
    return storage_native.getItem(key);
  }

  removeItem (key) {
    return storage_native.removeItem(key);
  }

  clear () {
    storage_native.clear();
  }

  get length () {
    return storage_native.length();
  }

  key (index) {
    return storage_native.key(index);
  }
}

exports.Storage = Storage;
