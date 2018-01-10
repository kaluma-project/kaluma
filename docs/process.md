Process
=======

* [Object: process]()
  * [arch]()
  * [platform]()
  * [version]()
  * [builtin_modules]()
  * [getBuiltinModule(builtin_module_name)]()
  * [binding(native_module_name)]()

## process


### arch

* `{string}`

-- ex) 'arm', 'x64', ...

### platform

* `{string}`

-- ex) 'linux', 'darwin', 'unknown', ...

### version

* `{string}`

-- semver format. ex) '0.1.0', ...

### builtin_modules

* `{Array<string>}`

array of builtin module names

### getBuiltinModule(builtin_module_name)

* __`builtin_module_name`__ `{string}`

load the builtin module and return 
the module as a function

### binding(native_module_name)

* __`native_module_name`__ `{string}`

load a native module

Has properties  <native_module_name> : binding has native module names as properties
