// TODO: Why global scope is invisible in board.js?
// TODO: variables becomes global (fs, VFSLittle, bd)

// initialize board object
global.board.name = "pico";
global.board.LED = 25;

// mount lfs on "/"
const fs = global.require("fs");
const { VFSLittleFS } = global.require("vfs_lfs");
fs.register("lfs", VFSLittleFS);
const bd = new global.Flash(0, 128);
fs.mount("/", bd, "lfs", true);
