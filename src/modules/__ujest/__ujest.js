/**
 * micro-Jest testing framework
 * written by Minkyu lee (@niklauslee)
 */

var tests = [];
var finished = [];
var timer = null;

function test(name, fn) {
  tests.push({
    name: name,
    fn: fn,
    state: 0, // 0=idle, 1=runs, 2=pass, 3=fail
    assertFail: 0
  });
}

function error(msg) {
  console.log(`\x1b[91m[Error] ${msg}\x1b[0m`);
}

function expect(v1) {
  var matchers = {
    toBe: function (v2) {
      if (!Object.is(v1, v2)) {
        tests[0].assertFail++;
        error(`${JSON.stringify(v2)} expected, but ${JSON.stringify(v1)} received`);
      }
    },
    notToBe: function (v2) {
      if (Object.is(v1, v2)) {
        tests[0].assertFail++;
        error(`${JSON.stringify(v2)} not expected, but ${JSON.stringify(v1)} received`);
      }
    },
    toBeTruthy: function () {
      if (!v1) {
        tests[0].assertFail++;
        error(`Truthy expected, but falsy (${JSON.stringify(v1)}) received`);        
      }
    },
    toBeFalsy: function () {
      if (v1) {
        tests[0].assertFail++;
        error(`Falsy expected, but truthy (${JSON.stringify(v1)}) received`);        
      }
    },
  };
  if (Array.isArray(v1)) {
    matchers = Object.assign(matchers, {
      toContain: function (v2) {
        if (v1.indexOf(v2) < 0) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to contain ${JSON.stringify(v2)}, but not contained`);
        }
      },
      notToContain: function (v2) {
        if (v1.indexOf(v2) >= 0) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected not to contain ${JSON.stringify(v2)}, but contained`);
        }
      }
    });
  }
  if (typeof v1 === 'number') {
    matchers = Object.assign(matchers, {
      toBeGreaterThan: function (v2) {
        if (v1 <= v2) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to be greater than ${JSON.stringify(v2)}, but not greater`);
        }
      },
      toBeGreaterThanOrEqual: function (v2) {
        if (v1 < v2) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to be greater than or equal to ${JSON.stringify(v2)}, but less`);
        }
      },
      toBeLessThan: function (v2) {
        if (v1 >= v2) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to be less than ${JSON.stringify(v2)}, but not less`);
        }
      },      
      toBeLessThanOrEqual: function (v2) {
        if (v1 > v2) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to be less than or equal to ${JSON.stringify(v2)}, but greater`);
        }
      },
    });
  }
  if (typeof v1 === 'string') {
    matchers = Object.assign(matchers, {
      toMatch: function (v2) { // v2 should be regex
        if (!v1.match(v2)) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} expected to match ${v2.toString()}, but not matched`);
        }
      },
      notToMatch: function (v2) { // v2 should be regex
        if (v1.match(v2)) {
          tests[0].assertFail++;
          error(`${JSON.stringify(v1)} not expected to match ${v2.toString()}, but matched`);
        }
      }
    });
  }
  if (typeof v1 === 'function') {
    matchers = Object.assign(matchers, {
      toThrow: function (v2) {
        try {
          v1();
          tests[0].assertFail++;
          error(`function expected to throw error, but not thrown`);
        } catch (err) {
        }
      },
      notToThrow: function (v2) {
        try {
          v1();
        } catch (err) {
          tests[0].assertFail++;
          error(`function expected not to throw error, but thrown`);
        }
      }
    });
  }
  return matchers;
}

/**
* Start all tests. If idx given, only the idx-th test to be performed
* @param {number} idx
*/
function start(idx) {
  if (typeof idx === 'number') {
    var _t = tests[idx];
    tests = [_t];
  }
  timer = setInterval(() => {
    if (tests.length > 0) {
      var t = tests[0];
      if (t.state === 0) { // idle
        t.state = 1;
        print(`\r\x1b[97;43m RUNS \x1b[0m ${t.name}`);
        try {
          t.fn((err) => {
            if (err) {
              t.state = 3;
              error(err.toString());
            } else {
              t.state = t.assertFail === 0 ? 2 : 3;
            }
          });
        } catch (e) {
          t.state = 3;
          console.log(e);
        }
      } else if (t.state === 2) { // pass
        print(`\x1b[2K\r\x1b[97;102m PASS \x1b[0m ${t.name}\r\n`);
        finished.push(tests.shift());
      } else if (t.state === 3) { // fail
        print(`\x1b[2K\r\x1b[97;101m FAIL \x1b[0m\x1b[91m ${t.name}\x1b[0m\r\n`);
        finished.push(tests.shift());
      }
    } else {
      clearInterval(timer);
      console.log('');
      var l = finished.length;
      var f = finished.filter(ts => ts.state === 3).length;
      var p = l - f;
      console.log(`Tests: ${p} passed, ${f} failed (${l} total)`);
    }
  }, 10);
}

exports.test = test;
exports.expect = expect;
exports.start = start;
