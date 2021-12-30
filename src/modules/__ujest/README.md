# Versions

- `1.0.0` The first release
- `1.1.0` Add `.toThrow()`, `.notToThrow()` matchers

# Overview

micro-Jest is a tiny JavaScript testing framework for Kaluma. It is distilled version of the original [Jest](https://jestjs.io/) project.

# Getting Started

Add `@niklauslee/micro-jest` dependency to your `package.json`.

```json
{
  dependencies: {
    "@niklauslee/micro-jest": <version>
  }
}
```

Here is a simple example. Different from Jest, you have to call `done()` always even if the test case is synchronous.

```js
const { test, expect, start } = require("@niklauslee/micro-jest");

function sum(a, b) {
  return a + b;
}

test("adds 1 + 2 to equal 3", (done) => {
  expect(sum(1, 2)).toBe(3);
  done(); // should be called
});

start(); // start to test
```

It is recommended to add a `test.js` file including test cases to your project. To run the `test.js` open and activate `test.js` file in IDE and select **Upload Current** menu.

# Using Matchers

Micro-Jest provides various matchers to test values. `toBe()` is used to test a value with exact equality.

```js
test("1 + 1 = 2", (done) => {
  expect(1 + 1).toBe(2);
  done();
});
```

Here is the matchers provided.

- `expect(value).toBe(value)`
- `expect(value).notToBe(value)`
- `expect(value).toBeTruthy()`
- `expect(value).toBeFalsy()`
- `expect(number).toBeGreaterThan(number)`
- `expect(number).toBeGreaterThanOrEqual(number)`
- `expect(number).toBeLessThan(number)`
- `expect(number).toBeLessThanOrEqual(number)`
- `expect(array).toContain(value)`
- `expect(array).notToContain(value)`
- `expect(string).toMatch(regex)`
- `expect(string).notToMatch(regex)`
- `expect(function).toThrow([message])`
- `expect(function).notToThrow([message])`

# Running tests

To run all test cases you have to call `jest.start()`.

However sometimes you want to run a particular test case rather than all test cases. You can do it by calling `start()` with a number of test case.

```javascript
start(0); // Run the first test case.
start(3); // Run the 4-th test case.
start(); // Run all the test cases.
```

It is convenient to add `global.start = start` in the end of the your test code (e.g., `test.js`). Then you can run all test or run a particular test in Terminal like `start()` or `start(3);`.

# Handling errors

If you catch an error you can pass it to `done()` callback.

```javascript
// async
test('adds 1 + 2 to equal 3', (done) => {
  someAsyncFunction(err => {
    if (err) {
      done(err);
    } else {
      expect(...).toBe(...);
      done();
    }
  })
});

// sync
test('adds 1 + 2 to equal 3', (done) => {
  try {
    someSyncFunction();
    done();
  } catch(err) {
    done(err);
  }
});
```
