// TODO: Why global scope is invisible in board.js?

global.board.name = 'pico';
global.board.LED = 25;

// mount lfs on "/"
/*
const fs = global.require('fs');
const {VFSLittleFS} = global.require('vfs_lfs');
const bd = new global.FlashBD(0, 64);
const vfs = new VFSLittleFS(bd);
fs.mount('/', vfs);
*/
