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

test("[storage] removeItem()", (done) => {
  storage.clear();
  // ...
  done();
});

start();
