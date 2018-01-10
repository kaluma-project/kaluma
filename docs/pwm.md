PWM (Pulse Width Modulation)
============================

* [Class: PWM]()
  * [setup(pin[, frequency[, duty]])]()
  * [start()]()
  * [stop()]()
  * [getFrequency()]()
  * [setFrequency(frequency)]()
  * [getDuty()]()
  * [setDuty(duty)]()
  * [close()]()

## Class: PWM

PWM Class.

```js
var PWM = require('pwm');
var pwm = new PWM(); // Get an instance of PWM
```

### setup(pin[, frequency[, duty]])

* `pin {number}` Pin number
* `freq {number}` in Hz
* `duty {number}` 0 ~ 1


```js
var PWM = require('pwm');
var pwm = new PWM();
pwm.setup(1, 1000, 0.5);
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
