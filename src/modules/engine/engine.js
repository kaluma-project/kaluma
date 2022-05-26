const { GPIO } = require("gpio");
const { screen } = require("screen");

let width = 0, height = 0;
let legend = {};
let img = { data: new Uint8Array(0), width: 0, height: 0 };
let currentLevel = [];
let maxTileDim;
let zOrder = [];
let solids = [];
let pushable = {};
let afterInputFn = () => {};
function clear() { currentLevel = []; }

exports.afterInput = fn => afterInputFn = fn;
exports.setSolids = arr => solids = arr;
exports.setZOrder = arr => zOrder = arr;
exports.setPushables = obj => pushable = obj;

const getGrid = exports.getGrid = () => {
	const overlaps = {};
	const tiles = currentLevel.map(tile => [ `${tile.x},${tile.y}`, tile ]);
	tiles.forEach( tile => {
		const [ key, data ] = tile;
		if (key in overlaps) overlaps[key].push(data);
		else overlaps[key] = [data];
	});

	return overlaps;
}

const canMoveToPush = (tile, dx, dy) => {
	const grid = getGrid();
	const { x, y, type } = tile;
	const cellKey = `${x+dx},${y+dy}`;

	const notSolid = !solids.includes(type);
	const noMovement = dx === 0 && dy === 0;
	const movingToEmpty = !grid[cellKey];

	if (notSolid || noMovement || movingToEmpty) {
		tile._x += dx;
		tile._y += dy;
		return true;
	}

	let canMove = true;

	grid[cellKey].forEach(cell => {
		const isSolid = solids.includes(cell.type);
		const isPushable = (type in pushable) && pushable[type].includes(cell.type);

		if (isSolid && !isPushable)
                    canMove = false;

		if (isSolid && isPushable) {
			canMove = canMove && canMoveToPush(cell, dx, dy);
		}
	})

	if (canMove) {
		tile._x += dx;
		tile._y += dy;
	}

	return canMove;
}

const swap = exports.swap = (arr, newTypes) => { // swap could do multiple
	if (typeof arr === "string") arr = [ arr ];
	if (typeof newTypes === "string") newTypes = [ newTypes ];

	const grid = getGrid();

	let matched = false;
	let length = 0;

	Object.keys(grid).forEach(k => {
		const cell = grid[k];
		const typesInCell = cell.map(tile => tile.type);

		const matches = [];

		arr.forEach(t => {
			const index = typesInCell.indexOf(t);
			if (index !== -1 && !matches.includes(index)) {
				matches.push(index);
			} 
		});

		if (matches.length === arr.length) {
			matches.forEach(i => cell[i].remove());
			const [ x, y ] = k.split(",").map(Number);

			newTypes.forEach(t => addTile(x, y, t));

			matched = true;
			length++;
		}
	})

	return length;
}

class Tile {
	constructor(x, y, type) {
		this._type = null;
		this.type = type;
		this._x = x;
		this._y = y;
		this.dx = 0;
		this.dy = 0;
	}

	set type(t) {
		if (t === ".") t.remove(); // hmm

		this._type = t;
		this.img = (t in legend) ? legend[t] : defaultSprite;
	}

	get type() {
		return this._type;
	}

	set x(newX) {
		const dx = newX - this.x;
		if (canMoveToPush(this, dx, 0)) this.dx = dx;
		return this;
	}

	get x() {
		return this._x;
	}

	set y(newY) {
		const dy = newY - this.y;
		if (canMoveToPush(this, 0, dy)) this.dy = dy;
		return this;
	}

	get y() {
		return this._y;
	}

	remove() {
		currentLevel = currentLevel.filter(t => t !== this);

		return this;
	}

}

exports.getTile = type => currentLevel.find(t => t.type === type), // **
exports.getAllTiles = type => currentLevel.filter(t => t.type === type); // **

const addTile = exports.addTile = (x, y, type) => { // could take array
	// if (type === ".") 

	const tile = new Tile(x, y, type);
	currentLevel.push(tile);

	return tile;
};


const dpad = {
  up:      { last: 0, pin: new GPIO(0, INPUT_PULLUP) },
  down:    { last: 0, pin: new GPIO(3, INPUT_PULLUP) },
  left:    { last: 0, pin: new GPIO(2, INPUT_PULLUP) },
  right:   { last: 0, pin: new GPIO(1, INPUT_PULLUP) },
  action0: { last: 0, pin: new GPIO(4, INPUT_PULLUP) },
  action1: { last: 0, pin: new GPIO(5, INPUT_PULLUP) },
};
exports.onInput = function onInput(key, handler) {
	if (!dpad[key]) throw new Error(
		`expected one of "up", "down", "left", or "right", found ${key}`
	);
	dpad[key].handler = handler;
}
const gc = screen.getContext("buffer");

class ImageData {
	constructor(pixels, w, h) {
		this.width = w;
		this.height = h;
		const bytes = new Uint8Array(w*h*2);
		for (let x = 0; x < w; x++)
			for (let y = 0; y < h; y++) {
				let i = (y*w + x);
				const [r, g, b, a] = pixels.slice(i*4, (i + 1)*4);
				if (a < 255) continue;
				const col = gc.color16(Math.max(b, 5),
									   Math.max(g, 5),
									   Math.max(r, 5));
				bytes[i*2+0] = col >> 8;
				bytes[i*2+1] = col;
			}
		this.data = new Uint16Array(bytes.buffer);
	}
}

(() => {
 const width = 128, height = 160;
 const pixels = new Uint16Array(width * height);
 const { rfill, sprdraw } = global.require("native");
 setInterval(() => {
   for (const [name, btn] of Object.entries(dpad)) {
     const { pin, handler } = btn;
     const now = pin.read();
     if (handler && btn.last != now && !(btn.last = now))
       handler();
   }
   afterInputFn();

   rfill(pixels, gc.color16(255, 255, 255), width*height);

   currentLevel
     .sort((a, b) => zOrder.indexOf(b.type) - zOrder.indexOf(a.type))
     .forEach(tile => {
       sprdraw(tile.img.data, tile.img.width, tile.img.height,
           pixels, tile.y*16, tile.x*16);
     });

   screen.fillImage(0, 0, width, height, new Uint8Array(pixels.buffer));
 }, 1000/20);
})();

const allEqual = arr => arr.every(val => val === arr[0]);
exports.sprite = function sprite(string) { // returns image data
	const rows = string.trim().split("\n").map(x => x.trim());
	const rowLengths = rows.map(x => x.length);
	const isRect = allEqual(rowLengths);
	if (!isRect) console.error("Level must be rect.");
	const width = rows[0].length;
	const height = rows.length;
	const data = new Uint8ClampedArray(16*16*4);

	const colors = {
		"0": [0, 0, 0, 255],
		"1": [255, 255, 255, 255],
		"r": [255, 0, 0, 255],
		"g": [0, 255, 0, 255],
		"b": [0, 0, 255, 255],
		".": [0, 0, 0, 0],
	}

	const chars = string.split("").filter(x => x.match(/\S/));
	for (let i = 0; i < 16*16; i++) {
		const type = chars[i % (width*height)];

		if (!(type in colors)) console.error("unknown color:", type);

		const [ r, g, b, a ] = colors[type];
		data[i*4] = r;
		data[i*4 + 1] = g;
		data[i*4 + 2] = b;
		data[i*4 + 3] = a;
	}

	const result = new ImageData(data, 16, 16);

	return img = result;
};

function parsePattern(string) {
    const parsedPattern = [];
    const rows = string.trim().split("\n").map(x => x.trim());
    const rowLengths = rows.map(x => x.length);
    const isRect = allEqual(rowLengths)
    if (!isRect) console.error("pattern must be rectangle");
    const w = rows[0].length;
    const h = rows.length;

    for (let i = 0; i < w*h; i++) {
        const type = string.split("").filter(x => x.match(/\S/))[i];
        parsedPattern.push(type)
    }

    const result = { width: w, height: h, pattern: parsedPattern };

    return result;
}

function matchPattern(patternData, testMap = {}) {
    const { width: w, height: h, pattern } = patternData;

    const grid = getGrid();

    // if no cell with key then cell empty
    for (let i = 0; i < width*height; i++) {
        const x = i%width; 
        const y = Math.floor(i/width); 
        const key = `${x},${y}`;


        if (!grid[key]) grid[key] = [{ x, y, type: "." }];
    }

    let allMatches = [];

    for (let i = 0; i < width*height; i++) {
      const x = i%width; 
      const y = Math.floor(i/width); 

      if (x + w > width || y + h > height) continue;
      
      let match = true;
      let matches = [];
      for (let j = 0; j < w*h; j++) {
        const dx = j%w; 
        const dy = Math.floor(j/w);
        const type = pattern[j];
        const key = `${x+dx},${y+dy}`;
        
        let testFn;
        if (type in testMap) {
          const val = testMap[type];
          if (Array.isArray(val)) testFn = t => val.includes(t.type);
          if (typeof val === "function") testFn = val
        }

        let matchValue = (testFn)
            ? grid[key].find(testFn) // could take whole tile or tile type
            : grid[key].find(t => t.type === type)

        match = match && matchValue !== undefined;

        matches.push(matchValue);
      }

      if (match) {
        // if match doesn't have overlap with existing matches
        const overlap = matches.some(t => allMatches.flat().includes(t));
        if (!overlap) allMatches.push(matches);
      }
    }

    return allMatches;
}

exports.match = function match(pattern, testMap = {}) {
    const p = parsePattern(pattern);
    const matches = matchPattern(p, testMap);
    return matches;
};

const setScreenSize = exports.setScreenSize = function() {}
exports.setLegend = function setLegend(newLegend) { legend = newLegend; }
exports.setMap = function setMap(string) { // could have background and sprites
	// check that level is rectangle

	clear();

	const rows = string.trim().split("\n").map(x => x.trim());
	const rowLengths = rows.map(x => x.length);
	const isRect = allEqual(rowLengths)
		if (!isRect) console.error("Level must be rect.");
	const w = rows[0].length;
	const h = rows.length;
	width = w;
	height = h;

	// scale the ctx based on aspect ratio of level
	// tiles should always be square
	// find max tile width to fit

	maxTileDim = Math.min(width/w, height/h);

	// should this adjust screen size?
	setScreenSize(w*maxTileDim, h*maxTileDim);

	const chars = string.split("").filter(x => x.match(/\S/));
	for (let i = 0; i < w*h; i++) {
		const type = chars[i];

		if (type === ".") continue;

		const x = i%w; 
		const y = Math.floor(i/w); 
		const newTile = new Tile(x, y, type);
		currentLevel.push(newTile)
	}

	return currentLevel;
}
