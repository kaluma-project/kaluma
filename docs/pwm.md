PWM (Pulse Width Modulation)
============================

* [Class: PWM]()
  * [new PWM(pin[, frequency[, duty]])]()
  * [start()]()
  * [stop()]()
  * [getFrequency()]()
  * [setFrequency(frequency)]()
  * [getDuty()]()
  * [setDuty(duty)]()
  * [close()]()

## Class: PWM

PWM Class.

### new PWM(pin[, frequency[, duty]])

* `pin {number}` Pin number.
* `freq {number}` Frequency in Hz. Default is 490.
* `duty {number}` Duty cycle between 0 and 1. Default is 1.

```js
var PWM = require('pwm');
var pwm = new PWM(1, 1000, 0.5);
pwm.start();
// ...
pwm.stop();
pwm.close();
```

### start()

Start


### stop()

Stop


### getFrequency()

* Returns: `{number}`


### setFrequency(frequency)

* __`frequency`__ `{number}`


### getDuty()

* Returns: `{number}`


### setDuty(duty)

* __`duty`__ `{number}`


### close()

Close
