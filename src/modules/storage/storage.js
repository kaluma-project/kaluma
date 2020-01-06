var storage_native = process.binding(process.binding.storage);

class Storage {
  setItem (key, value) {
    return storage_native.setItem(key, value.toString());
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
