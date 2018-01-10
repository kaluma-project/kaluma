CONTRIBUTING
============

## Development settings

Required packages:
* __GNU ARM Toolchain__ (6-2017-q1-update) : https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
* __ST-Link tool__ : https://github.com/texane/stlink
* __Node.js__ : https://nodejs.org

Recommended tools:
* Visual Studo Code: https://code.visualstudio.com/
* VSCode Extensions: `C/C++`, `Native Debug`, `cpplint`

## C coding styles

Follows the conventions from the [Google C/C++ style guide](https://google.github.io/styleguide/cppguide.html). Some keypoints and additional guidelines are enumerated below.

* Two spaces and no tabs for indentation.
* File names consists of lower cases `[a-z]`, numbers `[0-9]` and underscore `_`. (e.g. `events.h`, `repl_commands.c`).
* Function names consists of lower cases `[a-z]`, numbers `[0-9]` and underscore `_`. (e.g. `gpio_write`, `add_to_events`).
* Block bracing style follows [K&R Style](https://en.wikipedia.org/wiki/Indentation_style#K.26R).


## Build & Run

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
# build default target (stm32f4discovery)
$ make

# or specify target
$ make TARGET=kameleon-core
```

You can find `kameleon.elf`, `kameleon.bin` in `/build` folder.

3. Flash

If you are using `stm32f4discovery` target, then flash as below:

```sh
$ st-flash write build/kameleon.bin 0x8000000
# or
$ make flash
```

4. Connect via Terminal

You can use REPL via terminal program.

```sh
# Press RESET button on board

# for MacOS
$ screen /dev/tty.usbmodem1441 115200

# or for Linux
$ sudo screen /dev/ttyACM? 115200
```
