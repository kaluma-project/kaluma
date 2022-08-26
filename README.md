# Sprig OS

Sprig OS is a fork of Kaluma, a JavaScript runtime for the Raspberry Pi Pico

It adds the following three modules, which can be found in src/modules/ alongside the modules Kaluma provides natively
- "native," which contains the javascript game engine reimplemented in C for speed, as well as a software renderer for the games
- "engine," which is a thin layer of JavaScript wrapping around "native"
- "i2saudio," which contains our custom audio implementation (including some PIO assembly!)

[Learn more about sprig here!](https://sprig.hackclub.com)
