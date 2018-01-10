Timers
======

* [delay(msec)]()
* [millis()]()
* [setTimeout(callback, delay)]()
* [setInterval(callback, delay)]()
* [clearTimeout(id)]()
* [clearInterval(id)]()

## delay(msec)

* __`msec`__ `{number}`

```js
delay(3000); // delay 3 seconds.
```


## millis()

* Returns: `{number}` Milliseconds.


## setTimeout(callback, delay)

* __`callback`__ `{function()}`
* __`delay`__ `{number}`
* Returns: `{number}` Timer id.

```js
var timerId = setTimeout(function () {
  print('done.');
}, 1000);
```

## setInterval(callback, delay)

* __`callback`__ `{function()}`
* __`delay`__ `{number}`
* Returns: `{number}` Timer id.

```js
// Print "tick" for every seconds
var timerId = setInterval(function () {
  print('tick');
}, 1000);
// ...
clearInterval(timerId); // To stop printing "tick"
```

## clearTimeout(id)

* __`id`__ `{number}` Timer id.

```js
var timerId = setTimeout(...);
clearTimeout(timerId);
```

## clearInterval(id)

* __`id`__ `{number}` Timer id.
