// initialize board object
global.board.name = "pico2";
global.board.LED = 25;

// mount lfs on "/"
const fs = require("fs");
const { VFSLittleFS } = require("vfs_lfs");
const { Flash } = require("flash");
fs.register("lfs", VFSLittleFS);
// fs block starts after 16(storage) + 384(program)
const bd = new Flash(400, 384);
fs.mount("/", bd, "lfs", true);
