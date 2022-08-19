const { GPIO } = require("gpio");
const { screen } = require("screen");
const gc = screen.getContext("buffer");
const native = global.require("native");

/* ugh, eventually, I will, I promise!!! */
exports.addText = () => console.error;
exports.clearText = () => console.error("no text clear");

/* re-exports from C; src/modules/native/module_native.c has notes about why C */
exports.setMap = map => native.setMap(map.trim());
exports.addSprite = native.addSprite;
exports.getGrid = native.getGrid;
exports.getTile = native.getTile;
exports.tilesWith = native.tilesWith;
exports.clearTile = native.clearTile;
exports.getFirst = native.getFirst;
exports.getAll = native.getAll;
exports.width = native.width;
exports.height = native.height;
exports.setBackground = native.setBackground;

/* opts: x, y, color (all optional) */
exports.addText = (str, opts={}) => {
  const CHARS_MAX_X = 21;
  const padLeft = Math.floor((CHARS_MAX_X - str.length)/2);

  native.text_add(
    str,
    opts.color ?? [10, 10, 40],
    opts.x ?? padLeft,
    opts.y ?? 0
  );
}

exports.setLegend = (...bitmaps) => {
  native.legend_clear();
  for (const [charStr, bitmap] of bitmaps) {
    native.legend_doodle_set(charStr, bitmap.trim());
  }
  native.legend_prepare();
};

exports.setSolids = solids => {
  native.solids_clear();
  solids.forEach(native.solids_push);
};

exports.setPushables = pushTable => {
  native.push_table_clear();
  for (const [pusher, pushesList] of Object.entries(pushTable))
    for (const pushes of pushesList)
      native.push_table_set(pusher, pushes);
};

let afterInputs = [];
exports.afterInput = fn => afterInputs.push(fn);
exports.onInput = (() => {
  const { Button } = require('button');

  const dpad = {
    "w": { handlers: [], button: new Button(12, { debounce: 0 }) },
    "s": { handlers: [], button: new Button(14, { debounce: 0 }) },
    "a": { handlers: [], button: new Button(13, { debounce: 0 }) },
    "d": { handlers: [], button: new Button(15, { debounce: 0 }) },
    "i": { handlers: [], button: new Button( 5, { debounce: 0 }) },
    "k": { handlers: [], button: new Button( 8, { debounce: 0 }) },
    "j": { handlers: [], button: new Button( 6, { debounce: 0 }) },
    "l": { handlers: [], button: new Button( 7, { debounce: 0 }) },
  };
  for (const { handlers, button } of Object.values(dpad))
    button.on('click', () => {
      handlers.forEach(f => f());
      afterInputs.forEach(f => f());
      native.map_clear_deltas();
    });
  
  return (key, handler) => {
    if (!(key in dpad)) throw new Error(`the Sprig doesn't have a ${key} button ;)`);
    dpad[key].handlers.push(handler);
  };
})();

function _makeTag(cb) {
  return (strings, ...interps) => {
    if (typeof strings === "string") {
      throw new Error("Tagged template literal must be used like name`text`, instead of name(`text`)");
    }
    const string = strings.reduce((p, c, i) => p + c + (interps[i] ?? ''), '');
    return cb(string);
  }
}
exports.bitmap = _makeTag(text => text);
exports.tune = _makeTag(text => text);
exports.map = _makeTag(text => text);

setInterval(() => {
  /* I have no idea how this is going to hold up if you reload the module
     I'm not clear on when init gets called. */

  const width = 128, height = 160;
  const pixels = new Uint16Array(width * height);
  const { render } = native;
  
  pixels.fill(gc.color16(0, 0, 0));

  render(pixels, 0);
  screen.fillImage(0, 0, width, height, new Uint8Array(pixels.buffer));
}, 1000/20);
