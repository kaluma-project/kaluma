const sep = "/";
const delimiter = ":";

function isAbsolute(path) {
  return path.startsWith(sep);
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

function parse(path) {
  const _terms = path.split(sep).filter(t => t.length > 0);
  const _base = _terms.pop() || '';
  const _bt = _base.split('.');
  const _ext = _bt.pop() || '';
  const _name = _bt.join('.');
  let pathObj = {
    root: isAbsolute(path) ? sep : '',
    dir: _terms.join(sep),
    base: _base,
    name: _name,
    ext: '.' + _ext
  }
  return pathObj;
}

function dirname(path) {
  const _po = parse(path);
  return _po.root + _po.dir;
}

function extname(path) {
  return parse(path).ext;
}

function basename(path, ext) {
  const _po = parse(path);
  return _po.ext === ext ? _po.name : _po.base;
}

function normalize(path) {
  const _terms = path.split(sep).filter(t => t.length > 0);
  let _paths = [];
  if (path.startsWith(sep)) _paths.push('');
  for (let i = 0; i < _terms.length; i++) {
    const _t = _terms[i];
    if (_t === '.') {
      // do nothing
    } else if (_t === '..') {
      _paths.pop();
    } else {
      _paths.push(_t);
    }
  }
  return _paths.join(sep);
}

function join(...paths) {
  let _terms = [];
  if (paths.length > 0 && paths[0].startsWith(sep)) _terms.push('');
  for (let i = 0; i < paths.length; i++) {
    const _p = paths[i];
    _terms = _terms.concat(_p.split(sep).filter(t => t.length > 0));
  }
  return normalize(_terms.join(sep));
}

function resolve(...paths) {
  const _cwd = global.__cwd || sep;
  let _dir = _cwd;
  for (let i = 0; i < paths.length; i++) {
    const _path = paths[i];
    if (isAbsolute(_path)) {
      _dir = normalize(_path);
    } else {
      _dir = join(_dir, _path);
    }
  }
  return _dir;
}

exports.isAbsolute = isAbsolute;
exports.format = format;
exports.parse = parse;
exports.join = join;
exports.dirname = dirname;
exports.extname = extname;
exports.basename = basename;
exports.resolve = resolve;
exports.normalize = normalize;
exports.sep = sep;
exports.delimiter = delimiter;
