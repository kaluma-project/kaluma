const { GPIO } = require("gpio");
const { screen } = require("screen");
const gc = screen.getContext("buffer");
const native = global.require("native");

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

exports.clearText = () => native.text_clear();


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

{
  const tones = {
    "B0": 31,
    "C1": 33,
    "C#1": 35,
    "D1": 37,
    "D#1": 39,
    "E1": 41,
    "F1": 44,
    "F#1": 46,
    "G1": 49,
    "G#1": 52,
    "A1": 55,
    "A#1": 58,
    "B1": 62,
    "C2": 65,
    "C#2": 69,
    "D2": 73,
    "D#2": 78,
    "E2": 82,
    "F2": 87,
    "F#2": 93,
    "G2": 98,
    "G#2": 104,
    "A2": 110,
    "A#2": 117,
    "B2": 123,
    "C3": 131,
    "C#3": 139,
    "D3": 147,
    "D#3": 156,
    "E3": 165,
    "F3": 175,
    "F#3": 185,
    "G3": 196,
    "G#3": 208,
    "A3": 220,
    "A#3": 233,
    "B3": 247,
    "C4": 262,
    "C#4": 277,
    "D4": 294,
    "D#4": 311,
    "E4": 330,
    "F4": 349,
    "F#4": 370,
    "G4": 392,
    "G#4": 415,
    "A4": 440,
    "A#4": 466,
    "B4": 494,
    "C5": 523,
    "C#5": 554,
    "D5": 587,
    "D#5": 622,
    "E5": 659,
    "F5": 698,
    "F#5": 740,
    "G5": 784,
    "G#5": 831,
    "A5": 880,
    "A#5": 932,
    "B5": 988,
    "C6": 1047,
    "C#6": 1109,
    "D6": 1175,
    "D#6": 1245,
    "E6": 1319,
    "F6": 1397,
    "F#6": 1480,
    "G6": 1568,
    "G#6": 1661,
    "A6": 1760,
    "A#6": 1865,
    "B6": 1976,
    "C7": 2093,
    "C#7": 2217,
    "D7": 2349,
    "D#7": 2489,
    "E7": 2637,
    "F7": 2794,
    "F#7": 2960,
    "G7": 3136,
    "G#7": 3322,
    "A7": 3520,
    "A#7": 3729,
    "B7": 3951,
    "C8": 4186,
    "C#8": 4435,
    "D8": 4699,
    "D#8": 4978
  };

  const instrumentKey = {
    '~': 'sine',
    '-': 'square',
    '^': 'triangle',
    '/': 'sawtooth',
  };

  const reverseInstrumentKey = Object.fromEntries(Object.entries(instrumentKey).map(([k, v]) => [v, k]));

  function textToTune(text) {
    const elements = text.replace(/\s/g, '').split(',');
    const tune = [];

    for (const element of elements) {
      if (!element) continue;
      const [durationRaw, notesRaw] = element.split(':');
      const duration = Math.round(parseInt(durationRaw));
      const notes = (notesRaw || '').split('+').map((noteRaw) => {
        if (!noteRaw) return [];
        const [, pitchRaw, instrumentRaw, durationRaw] = noteRaw.match(/^(.+)([~\-^\/])(.+)$/);
        return [
          instrumentKey[instrumentRaw],
          isNaN(parseInt(pitchRaw, 10)) ? pitchRaw : parseInt(pitchRaw, 10),
          parseInt(durationRaw, 10)
        ];
      });
      tune.push([duration, ...notes].flat());
    }

    return tune;
  }

  function tuneToText(tune) {
    const groupNotes = (notes) => {
      const groups = [];
      for (let i = 0; i < notes.length; i++) {
        if (i % 3 === 0) {
          groups.push([notes[i]]);
        } else {
          groups[groups.length-1].push(notes[i]);
        }
      }
      return groups;
    }

    const notesToString = ([duration, ...notes]) => (
      notes.length === 0 
        ? duration 
        : `${duration}: ${groupNotes(notes).map(notesToStringHelper).join(' + ')}`
    )

    const notesToStringHelper = ([instrument, duration, note]) => (
        `${duration}${reverseInstrumentKey[instrument]}${note}`
      )

    return tune.map(notesToString).join(',\n');
  }

  const INSTRUMENTS = ["sine", "triangle", "square", "sawtooth"];

  const audio = require("i2saudio");
  function playTuneHelper(tune, number, playingRef) {
    for (let i = 0; i < tune.length*number; i++) {
      const index = i%tune.length;
      if (!playingRef.playing) break;
      const noteSet = tune[index];
      const sleepTime = noteSet[0];
      
      for (let j = 1; j < noteSet.length; j += 3) {
        const instrument = noteSet[j];
        const note = noteSet[j+1];
        const duration = noteSet[j+2];

        const f = typeof note === "string" 
          ? tones[note.toUpperCase()]
          : 2**((note-69)/12)*440;

        const i = INSTRUMENTS.indexOf(instrument);
        if ((i >= 0) && f !== undefined)
          audio.push_freq(f, i);
      }
      audio.wait(sleepTime);
    }
  }

  exports.playTune = function(tune, number = 1) {
    let playingRef = { playing: true };

    playTuneHelper(textToTune(tune), number, playingRef);

    return {
      end() {
        playingRef.playing = false;
      },
      isPlaying() {
        return playingRef.playing;
      },
    };
  }
}

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
