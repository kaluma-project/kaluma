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
