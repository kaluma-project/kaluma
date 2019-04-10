REPL Commands
=============

* `.echo [on|off]` -- Echo on/off
* `.reset` -- Reset JavaScript runtime context.
* `.mem` -- Print heap memory usage information (total available, used and peak).
* `.gc` -- Enforce garbage collection.
* `.flash` [options] -- Read or write data to the non-versatile flash memory.
  * option `-w` -- Write user code to flash via Ymodem.
  * option `-e` -- Erase the user code in flash.
  * option `-t` -- Get total size of flash for user code.
  * option `-s` -- Get size of user code.
  * option `-r` -- Print user code in textual format.
* `.load` -- Load and run Javascript program stored in flash memory.
* `.v` -- Print firmware version.
* `.firmup` -- Firmware update mode (bootloader).
* `.help` -- Print all available commands.
