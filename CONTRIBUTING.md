CONTRIBUTING
============

## Development settings

Required packages:
* __GNU ARM Toolchain__ (6-2017-q1-update) : https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

```bsh
# macOS install example

# download gcc-arm-none-eabi
$ mkdir /usr/local/gcc-arm
$ tar -xjf gcc-arm-none-eabi-6-2017-q1-update-mac.tar.bz2 -C /usr/local/gcc-arm

# Update .bash_profile
export PATH="/usr/local/gcc-arm/gcc-arm-none-eabi-6-2017-q1-update/arm-none-eabi/bin:/usr/local/gcc-arm/gcc-arm-none-eabi-6-2017-q1-update/bin:$PATH"

# Check install
$ arm-none-eabi-gcc --version
```

* __ST-Link tool__ : https://github.com/texane/stlink
* __CMake__
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
$ git clone https://github.com/kaluma-project/kaluma.git
$ cd kaluma
$ git submodule init
$ git submodule update
$ npm install
```

2. Build

```sh
# build default target (kameleon-core)
$ mkdir build
$ cd build
$ cmake ..
$ make

# or specify target
$ mkdir build
$ cd build
$ cmake .. -DTARGET=kameleon-core
$ make
```

You can find `kameleon-core.elf`, `kameleon-core.bin` in `/build` folder.

3. Flash

If you are using `kameleon-core` target, then flash as below:

4. Connect via Terminal

You can use REPL via terminal program.

```sh
# Press RESET button on board

# for MacOS
$ screen /dev/tty.usbmodem? 115200

# or for Linux
$ sudo screen /dev/ttyACM? 115200

# To quit the screen session, press ctrl+a, k, y.
```
