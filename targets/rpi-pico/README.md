Target: Raspberry Pi Pico
=========================

## Build

```sh
# assume at /kaluma
$ mkdir build # if not exist
$ cd build
$ cmake .. -DTARGET=rpi-pico
$ make
```

The `rpi-pico.uf2` will be created in the build folder.