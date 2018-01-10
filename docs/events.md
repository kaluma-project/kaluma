Events
======

* [Class: EventEmitter]()
  * [addListener(eventName, listener)]()
  * [emit(eventName[, ...args])]()
  * [on(eventName, listener)]()
  * [once(eventName, listener)]()
  * [removeAllListeners([eventName])]()
  * [removeListener(eventName, listener)]()


## Class: EventEmitter

```js
var EventEmitter = require('events').EventEmitter;
```

### addListener(eventName, listener)

* __`eventName`__ `{string}`
* __`listener`__ `{function}`

### emit(eventName[, ...args])

* __`eventName`__ `{string}`
* __`...args`__ `{any}`

### on(eventName, listener)

* __`eventName`__ `{string}`
* __`listener`__ `{function}`

### once(eventName, listener)

* __`eventName`__ `{string}`
* __`listener`__ `{function}`

### removeAllListeners([eventName])

* __`eventName`__ `{string}`

### removeListener(eventName, listener)

* __`eventName`__ `{string}`
* __`listener`__ `{function}`

