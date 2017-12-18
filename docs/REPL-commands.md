# REPL Commands

* `.echo <on|off>` -- echo on/off
* ~~`.clear` -- Reset REPL context.~~ (suspended)
* ~~`.exit` -- Exit the REPL.~~ (suspended)
* `.flash` -- Read or write data to the non-versatile flash memory.
  * option `-w` -- Write data to flash in hex format. Send `0x1a`(Ctrl+Z) to finish to write.
  * option `-e` -- Erase the flash data. 
  * option `-c` -- Get checksum of the data.
  * option `-t` -- Get total size of flash.
  * option `-s` -- Get data size in flash.
  * option `-r` -- Read data in textual format.
* `.load` -- Load and run Javascript program stored in flash memory.

## Deprecated

* ~~`.cd <path>` -- change cwd (current working directory)~~
* ~~`.mkdir <dir>` -- make dir~~
* ~~`.rmdir <dir>` -- delete dir~~
* ~~`.pwd` -- print cwd~~
* ~~`.ls` -- list files in cwd~~
* ~~`.rm <file>` -- delete the file of path~~
* ~~`.cat <file>` -- print content of file of path~~
* ~~`.stat <file>` -- stat the file~~
* ~~`.load <file>` -- Load the file to the current REPL session.~~
* ~~`.format` -- Format the flash memory by smartfs~~
