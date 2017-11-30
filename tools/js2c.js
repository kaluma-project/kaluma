// Generate c source from js files

const fs = require('fs-extra')
const path = require('path')
const childProcess = require('child_process')
const mustache = require('mustache')
const minimist = require('minimist')

var modulesPath = path.join(__dirname, '../src/js')
var wrappers = []
var snapshots = []

var argv = minimist(process.argv.slice(2))
var modules = argv.modules.trim().split(' ')

console.log('Generating modules for build...')
console.log()

modules.forEach(moduleName => {
  console.log('module: ' + moduleName)
  const src = path.join(modulesPath, moduleName + '.js')
  const wrapped = path.join(modulesPath, moduleName + '.wrapped')
  const snapshot = path.join(modulesPath, moduleName + '.snapshot')
  generateWrapper(src, wrapped)
  wrappers.push(wrapped)
  generateSnapshot(wrapped, snapshot)
  snapshots.push(snapshot)
  console.log()
})
generate();

function generateWrapper(src, dest) {
  const wrapper_header = '(function(exports, require, module) {\n'
  const wrapper_footer = '\n});\n'
  const data = fs.readFileSync(src, 'utf8')
  fs.writeFileSync(dest, wrapper_header + data + wrapper_footer, 'utf8')
}

function generateSnapshot(src, dest) {
  childProcess.spawnSync('deps/jerryscript/build/bin/jerry-snapshot', [ 'generate', src, '-o', dest ], { stdio: 'inherit' })
}

function removeWrappers() {
  wrappers.forEach(item => {
    fs.unlinkSync(item)
  })  
}

function removeSnapshots() {
  snapshots.forEach(item => {
    fs.unlinkSync(item)
  })
}

function generate() {
  const template_h = fs.readFileSync(__dirname + '/kameleon_js.h.mustache', 'utf8')
  const template_c = fs.readFileSync(__dirname + '/kameleon_js.c.mustache', 'utf8')
  var view = {
    modules: []
  }
  snapshots.forEach(snapshot => {
    var buffer = fs.readFileSync(snapshot)
    var hex = buffer.toString('hex')
    var segments = hex.match(/.{1,20}/g)
    var moduleView = {
      name: path.basename(snapshot, '.snapshot'),
      nameUC: path.basename(snapshot, '.snapshot').toUpperCase(),
      size: buffer.length,
      segments: []
    }
    segments.forEach((segment, index) => {
      var bytes = segment.match(/.{1,2}/g).map(item => ({ value: item }))
      if (index == segments.length - 1) {
        bytes[bytes.length - 1].last = true
      }
      moduleView.segments.push({ bytes: bytes })
    })
    view.modules.push(moduleView)
  })
  view.modules[view.modules.length - 1].lastModule = true
  var rendered_h = mustache.render(template_h, view)
  var rendered_c = mustache.render(template_c, view)
  var genPath = path.join(__dirname, '../src/gen')
  fs.ensureDirSync(genPath)
  fs.writeFileSync(path.join(genPath, 'kameleon_js.h'), rendered_h, 'utf8')
  fs.writeFileSync(path.join(genPath, 'kameleon_js.c'), rendered_c, 'utf8')
  removeWrappers()
  removeSnapshots()
}
