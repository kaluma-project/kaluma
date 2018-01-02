// Generate c source from js files

const fs = require('fs-extra')
const path = require('path')
const childProcess = require('child_process')
const mustache = require('mustache')
const minimist = require('minimist')
const magicStrings = require('./magic_strings')

var modulesPath = path.join(__dirname, '../src/modules')
var wrappers = []
var snapshots = []

// Parse modules for generate
var argv = minimist(process.argv.slice(2))

// modules
var modules = argv.modules.trim().split(' ')

// modules which can be imported by `require()`
var requireModules = []

// modules which has native C implementation
var nativeModules = []

generateAll();

function generateAll() {
  console.log('Generating modules for build...')
  console.log()
  generateSnapshots()
  generateSources()
  removeWrappers()
  removeSnapshots()
  magicStrings.generateMagicStrings(modules)
}

function generateSnapshots() {
  modules.forEach(moduleName => {
    console.log('module: ' + moduleName + ' ------------------------------')
    const modpath = path.join(modulesPath, moduleName)
    const config = JSON.parse(fs.readFileSync(path.join(modpath + '/module.json')))
    console.log(config)
    if (config.require) {
      requireModules.push(moduleName)
    }
    if (config.native) {
      nativeModules.push(moduleName)
    }
    const src = path.join(modpath, moduleName + '.js')
    const wrapped = path.join(modpath, moduleName + '.wrapped')
    const snapshot = path.join(modpath, moduleName + '.snapshot')
    createWrapper(src, wrapped)
    wrappers.push(wrapped)
    createSnapshot(wrapped, snapshot)
    snapshots.push(snapshot)
    console.log()
  })
}

function createWrapper(src, dest) {
  const wrapper_header = '(function(exports, require, module) {\n'
  const wrapper_footer = '\n});\n'
  const data = fs.readFileSync(src, 'utf8')
  fs.writeFileSync(dest, wrapper_header + data + wrapper_footer, 'utf8')
}

function createSnapshot(src, dest) {
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

function generateSources() {
  const template_h = fs.readFileSync(__dirname + '/kameleon_modules.h.mustache', 'utf8')
  const template_c = fs.readFileSync(__dirname + '/kameleon_modules.c.mustache', 'utf8')
  var view = {
    modules: [],
    requireModules: [],
    nativeModules: []
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

  requireModules.forEach(mod => {
    view.requireModules.push({
      name: mod,
      nameUC: mod.toUpperCase(),
      native: (nativeModules.indexOf(mod) >= 0) ? true : false
    })
  })
  view.requireModules[view.requireModules.length - 1].lastModule = true

  nativeModules.forEach(mod => {
    view.nativeModules.push({
      name: mod,
      nameUC: mod.toUpperCase()
    })
  })
  view.nativeModules[view.nativeModules.length - 1].lastModule = true

  var rendered_h = mustache.render(template_h, view)
  var rendered_c = mustache.render(template_c, view)
  var genPath = path.join(__dirname, '../src/gen')
  fs.ensureDirSync(genPath)
  fs.writeFileSync(path.join(genPath, 'kameleon_modules.h'), rendered_h, 'utf8')
  fs.writeFileSync(path.join(genPath, 'kameleon_modules.c'), rendered_c, 'utf8')
}
