# REPL Commands

* `.echo <on|off>` -- echo on/off
* `.clear` -- Reset REPL context.
* `.exit` -- Exit the REPL.
* `.flash` -- Write binary data in the hex format to the flash memory. Sending `0x1a` (Ctrl+Z) is to finish to write.
  * option `-e` -- Erase the flash data. 
  * option `-c` -- Return checksum of the data in flash.
  * option `-i` -- Return information of the data in flash. (Total size of flash and total size of data).
  * option `-r` -- Return the data in textual format.
  * option `-h` -- Return the data in hex format.

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
