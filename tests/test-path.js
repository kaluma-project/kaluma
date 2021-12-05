const {test, start, expect} = require('__test');
const path = require('path');

test('path.parse() === root', (done) => {
  expect(path.parse()).toBe('/');
  done(); // should be called
});

start(); // start to test
