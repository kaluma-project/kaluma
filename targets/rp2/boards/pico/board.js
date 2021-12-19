// initialize board object
global.board.name = "pico";
global.board.LED = 25;

// mount lfs on "/"
const fs = require("fs");
const { VFSLittleFS } = require("vfs_lfs");
fs.register("lfs", VFSLittleFS);
// start block is 4(storage) + 128(program)
const bd = new global.Flash(132, 128);
fs.mount("/", bd, "lfs", true);
