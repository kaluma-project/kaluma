Development settings
====================

Required packages:
* __GNU ARM Toolchain__ (6-2017-q1-update) : https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
* __ST-Link tool__ : https://github.com/texane/stlink
* __Node.js__ : https://nodejs.org

Recommended tools:
* Visual Studo Code: https://code.visualstudio.com/
* VSCode Extensions: `C/C++`, `Native Debug` 

Build & Run
===========

1. Checkout repository

```sh
$ git clone https://github.com/kameleon-project/kameleon.git
$ cd kameleon
$ git submodule init
$ git submodule update
```

2. Build

```sh
$ make
```

3. Flash

```sh
$ st-flash write build/kameleon-core.bin 0x8000000
```

4. Connect via Terminal

```sh
# Press RESET button on board
$ screen /dev/tty.usbmodem1 115200 # macOS
# or
$ sudo screen /dev/ttyACM0 115200 # Linux
```
