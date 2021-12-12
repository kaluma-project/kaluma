// TODO: Why global scope is invisible in board.js?

// initialize board object
global.board.name = 'pico';
global.board.LED = 25;

// mount lfs on "/"
const fs = global.require('fs');
const {VFSLittleFS} = global.require('vfs_lfs');
fs.register('lfs', VFSLittleFS);
const bd = new global.FlashBD(0, 128);
fs.mount('/', bd, 'lfs', true);
