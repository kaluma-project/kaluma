
/**
 * URL class.
 * @param {string} input
 */
class URL {
  constructor (input) {
    this.protocol = '';
    this.username = '';
    this.password = '';
    this.hostname = '';
    this.port = '';
    this.pathname = '';
    this.search = '';
    this.hash = '';
    this.searchParams = null;
    // protocol
    var _i0 = input.indexOf('://');
    if (_i0 > 0) {
      this.protocol = input.substr(0, _i0 + 1);
      var rest = input.substr(_i0 + 3);
      // origin and path
      var auth_host = rest;
      var path_hash = '';
      var _i1 = rest.indexOf('/');
      if (_i1 < 0) _i1 = rest.indexOf('?');
      if (_i1 < 0) _i1 = rest.indexOf('#');
      if (_i1 > -1) {
        auth_host = rest.substr(0, _i1);
        path_hash = rest.substr(_i1);
      }
      // auth and host
      var _i2 = auth_host.indexOf('@');
      var host = auth_host;
      if (_i2 > -1) {
        var auth = auth_host.substr(0, _i2);
        var _i3 = auth.indexOf(':') // username : password
        if (_i3 > -1) {
          this.username = auth.substr(0, _i3);
          this.password = auth.substr(_i3 + 1);
        } else {
          this.username = auth;
        }
        host = auth_host.substr(_i2 + 1);
      }
      // host (hostname:port)
      var _i4 = host.indexOf(':');
      if (_i4 > -1) {
        this.hostname = host.substr(0, _i4);
        this.port = host.substr(_i4 + 1);
      } else {
        this.hostname = host;
        this.port = '';
      }
      if (path_hash.length > 0) {
        // path and hash
        var _i5 = path_hash.indexOf('#'); // path # hash
        var path = path_hash;
        if (_i5 > -1) {
          path = path_hash.substr(0, _i5);
          this.hash = path_hash.substr(_i5 + 1);
        }
        // pathname and search
        var _i6 = path.indexOf('?');
        if (_i6 > -1) {
          this.pathname = path.substr(0, _i6);
          this.search = path.substr(_i6);
          this.searchParams = new URLSearchParams(this.search);
        } else {
          this.pathname = path;
        }
      }
    } else {
      throw TypeError("Invalid URL");
    }
  }

  get href () {
    return this.toString()
  }

  get host () {
    return this.port ? this.hostname + ':' + this.port : this.hostname;
  }

  get origin () {
    var auth = this.username + (this.password ? ':' + this.password : '');
    return this.protocol + '//' + (auth ? auth + '@' : '') + this.host;
  }

  toString () {
    var path = this.pathname;
    if (this.search) path += this.search;
    if (this.hash) path += '#' + this.hash;
    var href = this.origin + path;
    return href;
  }

  toJSON () {
    return this.toString();
  }
}

/**
 * URLSearchParams class
 */
class URLSearchParams {
  constructor (input) {
    this.pairs = [];
    if (input) {
      if (input.startsWith('?')) {
        input = input.substr(1);
      }
      var ps = input.split('&');
      ps.forEach(p => {
        if (p.indexOf('=') > -1) {
          var pair = p.split('=');
          pair = pair.map(i => decodeURIComponent(i));
          this.pairs.push(pair);
        }
      })
    }
  }

  append (name, value) {
    this.pairs.push([name, value]);
  }

  delete (name) {
    this.pairs = this.pairs.filter(p => p[0] !== name);
  }

  entries () {
    return this.pairs;
  }

  get (name) {
    var pair = this.pairs.find(p => p[0] === name);
    if (pair) {
      return pair[1];
    }
    return null;
  }

  getAll (name) {
    var values = []
    this.pairs.forEach(p => {
      if (p[0] === name) {
        values.push(p[1]);
      }
    })
    return values;
  }

  has (name) {
    return this.pairs.find(p => p[0] === name) !== undefined;
  }

  set (name, value) {
    var pair = this.pairs.find(p => p[0] === name);
    if (pair) {
      pair[1] = value;
      this.pairs = this.pairs.filter(p => p[0] !== name || p === pair);
    } else {
      this.append(name, value);
    }
  }

  keys () {
    return this.pairs.map(p => p[0]);
  }

  values () {
    return this.pairs.map(p => p[1]);
  }

  toString () {
    var encoded = this.pairs.map(p => `${encodeURIComponent(p[0])}=${encodeURIComponent(p[1])}`);
    return encoded.join('&');
  }
}

exports.URL = URL;
exports.URLSearchParams = URLSearchParams;
