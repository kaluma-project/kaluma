console.log('hello!');
setInterval(() => {
  console.log('tick!');
}, 1000);

const {VFSLittleFS} = require('vfs_lfs');
console.log(VFSLittleFS);