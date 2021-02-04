Target: Linux
=============

## Build

```sh
# assume at /kaluma
$ mkdir build # if not exist
$ cd build 
$ cmake .. -DTARGET=linux
$ make
```

The `linux.elf` will be created in the build folder.
You can run `linux.elf` in the linux machine

> The linux porting is in progress now. So the full function is not implemented yet.
