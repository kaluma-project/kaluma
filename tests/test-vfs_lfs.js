const {test, start, expect} = require('__test');

function sum(a, b) {
  return a + b;
}

test('adds 1 + 2 to equal 3', (done) => {
  expect(sum(1, 2)).toBe(3);
  done(); // should be called
});

start(); // start to test
