const sep = "/";
const delimiter = ":";

function isAbsolute(path) {
  return path.startsWith("/");
}

function format(pathObj) {
  pathObj = Object.assign(
    {
      root: "",
      dir: "",
      base: "",
      name: "",
      ext: "",
    },
    pathObj
  );
  let _base = pathObj.base ? pathObj.base : pathObj.name + pathObj.ext;
  return pathObj.dir
    ? `${pathObj.dir}${sep}${_base}`
    : `${pathObj.root}${_base}`;
}

function parse() {
  return sep;
}

function join() {}

function dirname() {}

function extname() {}

function basename() {}

function relative() {}

function resolve() {}

function normalize() {}

exports.isAbsolute = isAbsolute;
exports.format = format;
exports.parse = parse;
exports.join = join;
exports.dirname = dirname;
exports.extname = extname;
exports.basename = basename;
exports.relative = relative;
exports.resolve = resolve;
exports.normalize = normalize;
exports.sep = sep;
exports.delimiter = delimiter;
