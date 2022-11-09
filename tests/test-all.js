const childProcess = require("child_process");

function cmd(cmd, args) {
  childProcess.spawnSync(cmd, args, { stdio: "inherit" });
}

// cmd("../build/kaluma", ["stream.test.js"]);
// cmd("../build/kaluma", ["path.test.js"]);
// cmd("../build/kaluma", ["process.test.js"]);
// cmd("../build/kaluma", ["storage.test.js"]);
// cmd("../build/kaluma", ["flash.test.js"]);
// cmd("../build/kaluma", ["vfs_lfs.test.js"]);
// cmd("../build/kaluma", ["vfs_fat.test.js"]);
// cmd("../build/kaluma", ["fs.test.js"]);
cmd("../build/kaluma", ["net.test.js"]);
