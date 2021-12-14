// initialize board object
global.board.name = "pico";
global.board.LED = 25;

// mount lfs on "/"
const fs = require("fs");
const { VFSLittleFS } = require("vfs_lfs");
fs.register("lfs", VFSLittleFS);
const bd = new global.Flash(0, 128);
fs.mount("/", bd, "lfs", true);
