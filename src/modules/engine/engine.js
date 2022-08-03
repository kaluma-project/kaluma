const { GPIO } = require("gpio");
const { screen } = require("screen");
const gc = screen.getContext("buffer");

const img = new Uint16Array(32 * 32);
for (let y = 0; y < 32; y++)
  for (let x = 0; x < 32; x++) {
    const m = !!((x+y) % 4) * 255;
    img[y*32 + x] = gc.color16(m, m, m);
  }

(() => {
 const width = 128, height = 160;
 const pixels = new Uint16Array(width * height);
 const { rfill, sprdraw } = global.require("native");

 rfill(pixels, gc.color16(255, 255, 255), width*height);
 sprdraw(img, 32, 32, pixels, 0, 0);

 screen.fillImage(0, 0, width, height, new Uint8Array(pixels.buffer));
})();
