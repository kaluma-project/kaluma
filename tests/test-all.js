const childProcess = require("child_process");

function cmd(cmd, args) {
  childProcess.spawnSync(cmd, args, { stdio: "inherit" });
}

cmd("../build/kaluma", ['test-path.js']);
cmd("../build/kaluma", ['test-vfs_lfs.js']);
cmd("../build/kaluma", ['test-fs.js']);
