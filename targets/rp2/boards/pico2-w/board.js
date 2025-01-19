// initialize board object
global.board.name = "pico2-w";

// mount lfs on "/"
const fs = require("fs");
const { VFSLittleFS } = require("vfs_lfs");
const { Flash } = require("flash");
fs.register("lfs", VFSLittleFS);
// fs block starts after 16(storage) + 384(program)
const bd = new Flash(384, 384);
fs.mount("/", bd, "lfs", true);

// setup ieee80211 and network drivers
const { PicoCYW43WIFI, PicoCYW43Network } = require("pico_cyw43");
global.__ieee80211dev = new PicoCYW43WIFI();
global.__netdev = new PicoCYW43Network();
