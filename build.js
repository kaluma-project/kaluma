// Generate c source from js files

const fs = require("fs-extra");
const path = require("path");
const minimist = require("minimist");
const childProcess = require("child_process");

// Parse options
var argv = minimist(process.argv.slice(2));

// const target = argv.target || "rp2";
// const board = argv.board || "pico";
const buildPath = path.join(__dirname, "build");
const srcGenPath = path.join(__dirname, "src/gen");

if (argv.clean) {
  clean();
} else {
  build();
}

function clean() {
  fs.removeSync(buildPath);
  fs.removeSync(srcGenPath);
}

function build() {
  // ensure /build
  fs.ensureDirSync(buildPath);

  // execute cmake and make
  process.chdir(buildPath);
  const params = [".."];
  if (argv.target) params.push(`-DTARGET=${argv.target}`);
  if (argv.board) params.push(`-DBOARD=${argv.board}`);
  if (argv.modules) params.push(`-DMODULES=${argv.modules}`);
  if (argv.board === 'pico_w') {
    params.push('-DPICO_CYW43_SUPPORTED=1');
    params.push('-DPICO_BOARD=pico_w');
  }

  console.log(params)
  cmd("cmake", params);
  cmd("make");
  process.chdir(__dirname);
}

function cmd(cmd, args) {
  childProcess.spawnSync(cmd, args, { stdio: "inherit" });
}
