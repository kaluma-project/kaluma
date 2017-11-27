// Generate c source from js files

const fs = require('fs')
const path = require('path')
const childProcess = require('child_process')
const mustache = require('mustache')

var js_path = path.join(__dirname, '../src/js')
var files = fs.readdirSync(js_path)
var snapshots = []

files.forEach(file => {
  if (path.extname(file) === '.js') {
    const basename = path.basename(file, '.js')
    const src = path.join(js_path, file)
    const wrapped = path.join(js_path, basename + '.wrapped')
    const snapshot = path.join(js_path, basename + '.snapshot')
    generateWrapper(src, wrapped)
    generateSnapshot(wrapped, snapshot)
    snapshots.push(snapshot)
  }
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
  var rendered_h = mustache.render(template_h, view)
  var rendered_c = mustache.render(template_c, view)
  fs.writeFileSync(path.join(js_path, '../kameleon_js.h'), rendered_h, 'utf8')
  fs.writeFileSync(path.join(js_path, '../kameleon_js.c'), rendered_c, 'utf8')
  // console.log(rendered_h)
  // console.log(rendered_c)
}
