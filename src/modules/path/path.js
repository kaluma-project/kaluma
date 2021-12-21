const sep = "/";

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
  let base = pathObj.base ? pathObj.base : pathObj.name + pathObj.ext;
  return pathObj.dir
    ? `${pathObj.dir}${sep}${base}`
    : `${pathObj.root}${base}`;
}

function parse(path) {
  const terms = path.split(sep).filter((t) => t.length > 0);
  const base = terms.pop() || "";
  const baseTerms = base.split(".");
  const ext = baseTerms.pop() || "";
  const name = baseTerms.join(".");
  let pathObj = {
    root: isAbsolute(path) ? sep : "",
    dir: terms.join(sep),
    base: base,
    name: name,
    ext: "." + ext,
  };
  return pathObj;
}

function normalize(path) {
  const terms = path.split(sep).filter((t) => t.length > 0);
  let paths = [];
  if (path.startsWith(sep)) paths.push("");
  for (let i = 0; i < terms.length; i++) {
    const term = terms[i];
    if (term === ".") {
      // do nothing
    } else if (term === "..") {
      paths.pop();
    } else {
      paths.push(term);
    }
  }
  return paths.join(sep) || sep;
}

function join(...paths) {
  let terms = [];
  if (paths.length > 0 && paths[0].startsWith(sep)) terms.push("");
  for (let i = 0; i < paths.length; i++) {
    const _path = paths[i];
    terms = terms.concat(_path.split(sep).filter((t) => t.length > 0));
  }
  return normalize(terms.join(sep));
}

function resolve(...paths) {
  const fs = require("fs");
  const _cwd = fs.cwd() || sep;
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
exports.resolve = resolve;
exports.normalize = normalize;
exports.sep = sep;
