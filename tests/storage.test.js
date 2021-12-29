const { test, start, expect } = require("__ujest");

test("[storage] clear()", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  expect(storage.length).toBe(1);
  storage.clear();
  expect(storage.length).toBe(0);
  done();
});

test("[storage] setItem()", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  let val = storage.getItem("key1");
  expect(val).toBe("value1");
  expect(storage.length).toBe(1);
  done();
});

test("[storage] setItem() - long data", (done) => {
  storage.clear();
  const d = "0123456789abcdefghijklmnopqrstuvwxyz"; // 36

  // total length: 252 (OK)
  let k1 = d + d + d; // 108
  let v1 = d + d + d + d; // 144
  storage.setItem(k1, v1);
  expect(storage.getItem(k1)).toBe(v1);

  // total length: 254 (Overflow - MAX: 253)
  let k2 = "!!" + d + d + d; // 110
  let v2 = d + d + d + d; // 144
  expect(() => {
    storage.setItem(k2, v2);
  }).toThrow();
  expect(storage.getItem(k2)).toBe(null);
  expect(storage.length).toBe(1);
  done();
});

test("[storage] setItem() - overwrite", (done) => {
  storage.clear();
  let k1 = "key1";
  let v1 = "value1";
  storage.setItem(k1, v1);
  expect(storage.getItem(k1)).toBe(v1);
  expect(storage.length).toBe(1);

  // overwrite
  let v2 = "value2";
  storage.setItem(k1, v2);
  expect(storage.getItem(k1)).toBe(v2);
  expect(storage.length).toBe(1);
  done();
});

test("[storage] getItem()", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");
  let val1 = storage.getItem("key1");
  let val2 = storage.getItem("key2");
  let val3 = storage.getItem("key3");
  expect(val1).toBe("value1");
  expect(val2).toBe("value2");
  expect(val3).toBe("value3");
  expect(storage.length).toBe(3);
  done();
});

test("[storage] getItem() - key not found", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");

  // key not found, returns null
  expect(storage.getItem("key4")).toBe(null);
  expect(storage.length).toBe(3);
  done();
});

test("[storage] removeItem()", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");
  expect(storage.length).toBe(3);

  // remove "key1"
  storage.removeItem("key1");
  expect(storage.getItem("key1")).toBe(null);
  expect(storage.length).toBe(2);

  // remove "key2"
  storage.removeItem("key2");
  expect(storage.getItem("key2")).toBe(null);
  expect(storage.length).toBe(1);

  // remove "key3"
  storage.removeItem("key3");
  expect(storage.getItem("key3")).toBe(null);
  expect(storage.length).toBe(0);
  done();
});

test("[storage] removeItem() - key not existed", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");
  expect(storage.length).toBe(3);

  // try to remove "key0" not existed (nothing happen)
  storage.removeItem("key0");
  expect(storage.length).toBe(3);
  done();
});

test("[storage] key()", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");
  expect(storage.length).toBe(3);

  let keys = ["key1", "key2", "key3"];
  expect(keys.length).toBe(3);

  // key(0)
  keys.splice(keys.indexOf(storage.key(0)), 1);
  expect(keys.length).toBe(2);

  // key(1)
  keys.splice(keys.indexOf(storage.key(1)), 1);
  expect(keys.length).toBe(1);

  // key(2)
  keys.splice(keys.indexOf(storage.key(2)), 1);
  expect(keys.length).toBe(0);
  done();
});

test("[storage] key() - range overflow", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  storage.setItem("key2", "value2");
  storage.setItem("key3", "value3");
  expect(storage.length).toBe(3);

  expect(storage.key(100)).toBe(null);
  done();
});

test("[storage] length", (done) => {
  storage.clear();
  storage.setItem("key1", "value1");
  expect(storage.length).toBe(1);
  storage.setItem("key2", "value2");
  expect(storage.length).toBe(2);
  storage.setItem("key3", "value3");
  expect(storage.length).toBe(3);
  done();
});

test("[storage] length - max overflow", (done) => {
  const MAX = 64;

  // push to max
  storage.clear();
  for (let i = 0; i < MAX; i++) {
    try {
      storage.setItem(`key-${i}`, "value data...");
    } catch (err) {
      console.log(err);
    }
  }
  // console.log(storage.length);
  // console.log(storage.key(0));
  expect(storage.length).toBe(MAX);

  // now, overflow
  expect(() => {
    storage.setItem("key", "value");
  }).toThrow();

  done();
});

test("[storage] length - sweeping", (done) => {
  const MAX = 64;

  // push to max
  storage.clear();
  for (let i = 0; i < MAX; i++) {
    try {
      storage.setItem(`key-${i}`, "value data...");
    } catch (err) {
      console.log(err);
    }
  }
  expect(storage.length).toBe(MAX);

  // remove one
  storage.removeItem("key-0");

  // add one
  storage.setItem("new-key", "new-value");
  expect(storage.getItem("new-key")).toBe("new-value");
  done();
});

start();
