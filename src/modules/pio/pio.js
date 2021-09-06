/**
 * PIO (Programmable I/O) class
 */

 class PIO {
  constructor(block) {
    this.block = block;
  }
}

class ASM {
  constructor() {
    this.code = [];
    this.labels = {};
    this.jmps = [];
  }

  jmp(cond, target) {
    let c = ASM.JMP;
    switch (cond) {
      case null: break;
      case '': break;
      case '!x': c |= 1 << 5; break;
      case 'x--': c |= 2 << 5; break;
      case '!y': c |= 3 << 5; break;
      case 'y--': c |= 4 << 5; break;
      case 'x!=y': c |= 5 << 5; break;
      case 'pin': c |= 6 << 5; break;
      case '!osre': c |= 7 << 5; break;
      default: throw new Error('Unknown condition of jmp()');
    }
    this.jmps.push({
      offset: this.code.length,
      target: target
    });
    this.code.push(c);
    return this;
  }

  wait(pol, src, idx) {
    let c = ASM.WAIT;
    if (pol) c |= 1 << 7;
    switch (src) {
      case 'gpio': c |= 0 << 5; break;
      case 'pin': c |= 1 << 5; break;
      case 'irq': c |= 2 << 5; break;
      // case reserved: c |= 3 << 5; break;
      default: throw new Error('Unknown source of wait()');
    }
    c |= idx;
    this.code.push(c);
    return this;
  }

  in(src, bits) {
    let c = ASM.IN;
    switch (src) {
      case 'pins': c |= 0 << 5; break;
      case 'x': c |= 1 << 5; break;
      case 'y': c |= 2 << 5; break;
      case 'null': c |= 3 << 5; break;
      // reserved: c |= 4 << 5; break;
      // reserved: c |= 5 << 5; break;
      case 'isr': c |= 6 << 5; break;
      case 'osr': c |= 7 << 5; break;
      default: throw new Error('Unknown source of in()');
    }
    c |= bits;
    this.code.push(c);
    return this;
  }

  out(dst, bits) {
    let c = ASM.OUT;
    switch (dst) {
      case 'pins': c |= 0; break;
      case 'x': c |= 1 << 5; break;
      case 'y': c |= 2 << 5; break;
      case 'null': c |= 3 << 5; break;
      case 'pindirs': c |= 4 << 5; break;
      case 'pc': c |= 5 << 5; break;
      case 'isr': c |= 6 << 5; break;
      case 'exec': c |= 7 << 5; break;
      default: throw new Error('Unknown destination of out()');
    }
    c |= bits;
    this.code.push(c);
    return this;
  }

  push(iffull, block = 1) {
    let c = ASM.PUSH;
    if (iffull) c |= 0x0040;
    if (block) c |= 0x0020;
    this.code.push(c);    
    return this;
  }

  pull(ifempty, block = 1) {
    let c = ASM.PULL;
    if (ifempty) c |= 0x0040;
    if (block) c |= 0x0020;
    this.code.push(c);
    return this;
  }

  mov(dst, src) {
    let c = ASM.MOV;
    switch (dst) {
      case 'pins': c |= 0 << 5; break;
      case 'x': c |= 1 << 5; break;
      case 'y': c |= 2 << 5; break;
      // reserved: c |= 3 << 5; break;
      case 'exec': c |= 4 << 5; break;
      case 'pc': c |= 5 << 5; break;
      case 'isr': c |= 6 << 5; break;
      case 'osr': c |= 7 << 5; break;
      default: throw new Error('Unknown destination of mov()');
    }
    // operation
    if (src.startsWith('~') || src.startsWith('!')) { // invert
      c |= 1 << 3;
      src = src.substr(1);
    }
    if (src.startsWith('::')) { // bit-reverse
      c |= 2 << 3;
      src = src.substr(2);
    }
    switch (src) {
      case 'pins': c |= 0; break;
      case 'x': c |= 1; break;
      case 'y': c |= 2; break;
      case 'null': c |= 3;
      // reserved: c |= 4;
      case 'status': c |= 5; break;
      case 'isr': c |= 6; break;
      case 'osr': c |= 7; break;
      default: throw new Error('Unknown source of mov()');
    }
    this.code.push(c);
    return this;
  }

  irq(wait, clear, irqnum, rel) {
    let c = ASM.IRQ;
    if (wait) c |= 1 << 5;
    if (clear) c |= 1 << 6;
    // TODO: irqnum, rel
    this.code.push(c);
    return this;
  }

  set(dst, val) {
    let c = ASM.SET;
    switch (dst) {
      case 'pins': c |= 0 << 5; break;
      case 'x': c |= 1 << 5; break;
      case 'y': c |= 2 << 5; break;
      // reserved: c |= 3 << 5; break;
      case 'pindirs': c |= 4 << 5; break;
      // reserved: c |= 5 << 5; break;
      // reserved: c |= 6 << 5; break;
      // reserved: c |= 7 << 5; break;
      default: throw new Error('Unknown destination of set()');
    }
    c |= val;
    this.code.push(c);
    return this;
  }

  // additional
  label(name) {
    this.labels[name] = this.code.length;
    return this;
  }

  wrap_target() {}
  wrap() {}

  sideset() {}
  delay() {}

  end () {
    // update all jmp
    this.jmps.forEach(jmp => {
      let c = this.code[jmp.offset];
      c |= this.labels[jmp.target]
      this.code[jmp.offset] = c;
    });
  }

  toBinary () {
    return new Uint16Array(this.code);
  }

  _print() {
    console.log(`[${this.code.map(c => c.toString(16)).join(', ')}]`);
  }

  // instruction
  static JMP = 0x0000;
  static WAIT = 0x2000;
  static IN = 0x4000;
  static OUT = 0x6000;
  static PUSH = 0x8000;
  static PULL = 0x8080;
  static MOV = 0xa000;
  static IRQ = 0xc000;
  static SET = 0xe000;

  // destination
  static DST_PINS = 0; // PINS
  static DST_X = 1; // X
  static DST_Y = 2; // Y
  static DST_NULL = 3; // NULL
  static DST_PINDIRS = 4; // PINDIRS
  static DST_PC = 5; // PC
  static DST_ISR = 6; // ISR
  static DST_EXEC = 7; // EXEC

  // condition
  static CND_ALWAYS = 0;
  static CND_X_ZERO = 1; // !X
  static CND_X_DEC = 2; // X--
  static CND_Y_ZERO = 3; // !Y
  static CND_Y_DEC = 4; // Y--
  static CND_NEQ_XY = 5; // X!=Y
  static CND_PIN = 6; // PIN
  static CND_OSRE_NEM = 0; // !OSRE
}

class StateMachine {
  constructor(pio, id, options) {}
  active(value) {}
  push() {}
  pull() {}
}

/*
const {PIO, ASM, StateMachine} = require('pio');
const asm = new ASM();
asm
  .push(...)
  .set(...)
  .jmp(...)
  ...
  .end()

const pio = new PIO(0, asm);
const sm = new StateMachine(pio, 0, options);
sm.push(1);
sm.push(2);
sm.pull();
...
*/

/* --------------------- examples ------------------------- */

// hello.pio
const hello_asm = new ASM();
hello_asm
.label('loop')
  .pull()
  .out('pins', 1)
  .jmp(null, 'loop')
  .end()
hello_asm._print();

// addition.pio
const addition_asm = new ASM();
addition_asm
  .pull()
  .mov('x', '~osr')
  .pull()
  .mov('y', 'osr')
  .jmp(null, 'test')
.label('incr')
  .jmp('x--', 'test')
.label('test')
  .jmp('y--', 'incr')
  .mov('isr', '~x')
  .push()
  .end()
addition_asm._print();

// clocked-input.pio
const clocked_input_asm = new ASM();
clocked_input_asm
  .wait(0, 'pin', 1)
  .wait(1, 'pin', 1)
  .in('pins', 1)
  .end()
clocked_input_asm._print();
