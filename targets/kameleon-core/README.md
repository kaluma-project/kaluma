# Target: Kameleon Core

## Build

```sh
$ node build --target=kameleon-core
```

The `kameleon-core.bin` will be created in the `/build` folder.

## Flash

Install [ST-Link](https://github.com/texane/stlink) tool first. Then execute `st-flash` tool as below:

```sh
$ st-flash write build/kameleon-core.bin 0x8000000
```

## TIP: DFU in macOS

- Install [`dfu-util`](http://dfu-util.sourceforge.net/)

```sh
$ brew install dfu-util
```

- Connect board in DFU mode (BOOT0 -> 3V3)
- Upload .dfu file. ([ref](http://dfu-util.gnumonks.org/dfuse.html))

```sh
% dfu-util -a 0 -D kameleon-1.0.0.dfu
```
