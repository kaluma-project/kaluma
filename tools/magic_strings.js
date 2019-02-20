// Generate magic strings

const fs = require('fs-extra')
const path = require('path')
const mustache = require('mustache')

var includePath = path.join(__dirname, '../include')
var modulesPath = path.join(__dirname, '../src/modules')

var magicStringHeaders = [ includePath + '/magic_strings.h' ]
var magicStrings = [];

function generateMagicStrings(modules) {
  // Extract magic string from all modules
  var headers = [ includePath + '/magic_strings.h' ]
  modules.forEach(mod => {
    var header = modulesPath + '/' + mod.name + '/' + mod.name + '_magic_strings.h'
    if (fs.existsSync(header)) {
      headers.push(header);
    }
  })
  headers.forEach(header => {
    extractMagicStrings(header);
  })
  // Sort magic strings by length and lexicographic
  magicStrings.sort(function (a, b) {
    if (a.length < b.length) {
      return -1;
    } else if (a.length > b.length) {
      return 1;
    } else {
      if (a < b) {
        return -1;
      } else if (a > b) {
        return 1;
      } else {
        return 0;
      }
    }
  });
  // Generate magic strings via templates
  magicStringItems = magicStrings.map(item => { return { id: item, len: item.length } })
  magicStringItems[magicStringItems.length - 1].last = true;

  const template_h = fs.readFileSync(__dirname + '/kameleon_magic_strings.h.mustache', 'utf8')
  var rendered_h = mustache.render(template_h, { magicStrings: magicStringItems })
  const template_c = fs.readFileSync(__dirname + '/kameleon_magic_strings.c.mustache', 'utf8')
  var rendered_c = mustache.render(template_c, { magicStrings: magicStringItems })

  var genPath = path.join(__dirname, '../src/gen')
  fs.ensureDirSync(genPath)
  fs.writeFileSync(path.join(genPath, 'kameleon_magic_strings.h'), rendered_h, 'utf8')
  fs.writeFileSync(path.join(genPath, 'kameleon_magic_strings.c'), rendered_c, 'utf8')
}

function extractMagicStrings(filePath) {
  var contents = fs.readFileSync(filePath, 'utf8');
  var lines = contents.split("\n");
  lines.forEach(line => {
    var str = line.trim();
    if (str.startsWith('#define MSTR_')) {
      var tokens = line.split(' ');
      if (tokens[1].startsWith('MSTR_') && tokens[2].startsWith('"')) {
        var item = tokens[2].substring(1, tokens[2].length - 1);
        if (!magicStrings.includes(item)) {
          magicStrings.push(item);
        }
      }
    }
  });
}

exports.generateMagicStrings = generateMagicStrings;
