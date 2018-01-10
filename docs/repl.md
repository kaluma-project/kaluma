REPL Commands
=============

* `.echo [on|off]` -- echo on/off
* `.clear` -- Reset Javascript context.
* `.mem` -- Print heap memory usage information (total available, used and peak).
* `.gc` -- Enforce garbage collection.
* `.flash` [options] -- Read or write data to the non-versatile flash memory.
* option `-w` -- Write data to flash in hex format. Send `0x1a`(Ctrl+Z) to finish to write.
* option `-e` -- Erase the flash data.
* option `-c` -- Get checksum of the data.
* option `-t` -- Get total size of flash.
* option `-s` -- Get data size in flash.
* option `-r` -- Read data in textual format.
* `.load` -- Load and run Javascript program stored in flash memory.
* `.help` -- Show all available commands.
