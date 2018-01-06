![logo](https://github.com/kameleon-project/kameleon/blob/master/logo.png?raw=true)

Overview
========

Kameleon is a tiny and efficient Javascript runtime for microcontrollers. The main features are:

* Small footprint (Run on 300KB ROM, 64KB RAM)
* ECMAScript 5.1/2015(subset) standard compliant (built on Jerryscript)
* Non-blocking I/O
* REPL mode support
* Portability (No RTOS)
* Native modules (File system, Networking, etc.)

Documentation
=============

* Getting Started
* API Reference
* Contributing

Supported Targets
=================

* ~~Kameleon Core~~ (to be released)
* STM32F4-Discovery


Development settings
====================

Required packages:
* __GNU ARM Toolchain__ (6-2017-q1-update) : https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
* __ST-Link tool__ : https://github.com/texane/stlink
* __Node.js__ : https://nodejs.org

Recommended tools:
* Visual Studo Code: https://code.visualstudio.com/
* VSCode Extensions: `C/C++`, `Native Debug`, `cpplint`

Build & Run
===========

1. Checkout repository

```sh
$ git clone https://github.com/kameleon-project/kameleon.git
$ cd kameleon
$ git submodule init
$ git submodule update
$ npm install
```

2. Build

```sh
$ make
```

3. Flash

```sh
$ st-flash write build/kameleon-core.bin 0x8000000
# or
$ make flash
```

4. Connect via Terminal

```sh
# Press RESET button on board
$ screen /dev/tty.usbmodem1441 115200 # macOS
# or
$ sudo screen /dev/ttyACM? 115200 # Linux
```
