// initialize board object
global.board.name = "pico";
global.board.LED = 25;

// mount lfs on "/"
const fs = require("fs");
const { VFSLittleFS } = require("vfs_lfs");
const { Flash } = require("flash");
fs.register("lfs", VFSLittleFS);
// fs block starts after 4(storage) + 128(program)
const bd = new Flash(132, 128);
fs.mount("/", bd, "lfs", true);
