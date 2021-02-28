# Target: Raspberry Pi Pico

## Setup

Setup steps required to build:

```sh
# clone repo
$ git clone https://github.com/kaluma-project/kaluma.git
$ cd kaluma

# init & update git submodules
$ git submodule update --init
$ cd lib/pico-sdk
$ git subomdule update --init # for pico-sdk
$ cd ..

## install npm modules
$ npm install
```

## Build

Build using a `build.js` script:

```sh
$ node build
# or, explicit --target option (rpi-pico is default)
$ node build --target=rpi-pico

# clean all builds
$ node build --clean
```

Alternatively use CMAKE:

```sh
$ mkdir build # if not exist
$ cd build
$ cmake .. -DTARGET=rpi-pico
$ make
```

The `rpi-pico.uf2` will be created in the `/build` folder.
