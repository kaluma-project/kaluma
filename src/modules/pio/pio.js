const pio_native = process.binding(process.binding.pio);

const PIO = {
  FIFO_JOIN_NONE: 0,
  FIFO_JOIN_TX: 1,
  FIFO_JOIN_RX: 2,
  SHIFT_LEFT: 0,
  SHIFT_RIGHT: 1,
  TX_LESSTHAN: 0,
  RX_LESSTHAN: 1,
};

class ASM {
  constructor(options) {
    this.code = [];
    this.labels = {};
    this.jmps = [];
    options = Object.assign(
      {
        sideset: 0,
      },
      options
    );
    this.sideset = options.sideset;
  }

  jmp(cond, target) {
    let c = ASM.JMP;
    if (arguments.length < 2) {
      target = cond;
      cond = null;
    }
    switch (cond) {
      case null:
        break;
      case "":
        break;
      case "!x":
        c |= 1 << 5;
        break;
      case "x--":
        c |= 2 << 5;
        break;
      case "!y":
        c |= 3 << 5;
        break;
      case "y--":
        c |= 4 << 5;
        break;
      case "x!=y":
        c |= 5 << 5;
        break;
      case "pin":
        c |= 6 << 5;
        break;
      case "!osre":
        c |= 7 << 5;
        break;
      default:
        throw new Error("Unknown condition of jmp()");
    }
    this.jmps.push({
      offset: this.code.length,
      target: target,
    });
    this.code.push(c);
    return this;
  }

  wait(pol, src, idx) {
    let c = ASM.WAIT;
    if (pol) c |= 1 << 7;
    switch (src) {
      case "gpio":
        c |= 0 << 5;
        break;
      case "pin":
        c |= 1 << 5;
        break;
      case "irq":
        c |= 2 << 5;
        break;
      // case reserved: c |= 3 << 5; break;
      default:
        throw new Error("Unknown source of wait()");
    }
    c |= idx;
    this.code.push(c);
    return this;
  }

  in(src, bits) {
    let c = ASM.IN;
    switch (src) {
      case "pins":
        c |= 0 << 5;
        break;
      case "x":
        c |= 1 << 5;
        break;
      case "y":
        c |= 2 << 5;
        break;
      case "null":
        c |= 3 << 5;
        break;
      // reserved: c |= 4 << 5; break;
      // reserved: c |= 5 << 5; break;
      case "isr":
        c |= 6 << 5;
        break;
      case "osr":
        c |= 7 << 5;
        break;
      default:
        throw new Error("Unknown source of in()");
    }
    c |= bits;
    this.code.push(c);
    return this;
  }

  out(dst, bits) {
    let c = ASM.OUT;
    switch (dst) {
      case "pins":
        c |= 0;
        break;
      case "x":
        c |= 1 << 5;
        break;
      case "y":
        c |= 2 << 5;
        break;
      case "null":
        c |= 3 << 5;
        break;
      case "pindirs":
        c |= 4 << 5;
        break;
      case "pc":
        c |= 5 << 5;
        break;
      case "isr":
        c |= 6 << 5;
        break;
      case "exec":
        c |= 7 << 5;
        break;
      default:
        throw new Error("Unknown destination of out()");
    }
    c |= bits;
    this.code.push(c);
    return this;
  }

  push(iffull, block = 1) {
    let c = ASM.PUSH;
    if (iffull === 1 || iffull === "iffull") c |= 0x0040;
    if (block === 1 || block === "block") c |= 0x0020;
    this.code.push(c);
    return this;
  }

  pull(ifempty, block = 1) {
    let c = ASM.PULL;
    if (ifempty === 1 || ifempty === "ifempty") c |= 0x0040;
    if (block === 1 || block === "block") c |= 0x0020;
    this.code.push(c);
    return this;
  }

  mov(dst, src) {
    let c = ASM.MOV;
    switch (dst) {
      case "pins":
        c |= 0 << 5;
        break;
      case "x":
        c |= 1 << 5;
        break;
      case "y":
        c |= 2 << 5;
        break;
      // reserved: c |= 3 << 5; break;
      case "exec":
        c |= 4 << 5;
        break;
      case "pc":
        c |= 5 << 5;
        break;
      case "isr":
        c |= 6 << 5;
        break;
      case "osr":
        c |= 7 << 5;
        break;
      default:
        throw new Error("Unknown destination of mov()");
    }
    // operation
    if (src.startsWith("~") || src.startsWith("!")) {
      // invert
      c |= 1 << 3;
      src = src.substr(1);
    }
    if (src.startsWith("::")) {
      // bit-reverse
      c |= 2 << 3;
      src = src.substr(2);
    }
    switch (src) {
      case "pins":
        c |= 0;
        break;
      case "x":
        c |= 1;
        break;
      case "y":
        c |= 2;
        break;
      case "null":
        c |= 3;
      // reserved: c |= 4;
      case "status":
        c |= 5;
        break;
      case "isr":
        c |= 6;
        break;
      case "osr":
        c |= 7;
        break;
      default:
        throw new Error("Unknown source of mov()");
    }
    this.code.push(c);
    return this;
  }

  irq(cmd, irqnum, rel) {
    let c = ASM.IRQ;
    let wait = 0;
    let clear = 0;
    if (typeof cmd === "number") {
      rel = irqnum;
      irqnum = cmd;
      cmd = null;
    }
    switch (cmd) {
      case null:
        break;
      case "set":
        break;
      case "nowait":
        break;
      case "wait":
        wait = 1;
        break;
      case "clear":
        clear = 1;
        break;
    }
    if (wait) c |= 1 << 5;
    if (clear) c |= 1 << 6;
    c |= irqnum;
    if (rel === "rel") {
      c |= 1 << 4;
    }
    this.code.push(c);
    return this;
  }

  set(dst, val) {
    let c = ASM.SET;
    switch (dst) {
      case "pins":
        c |= 0 << 5;
        break;
      case "x":
        c |= 1 << 5;
        break;
      case "y":
        c |= 2 << 5;
        break;
      // reserved: c |= 3 << 5; break;
      case "pindirs":
        c |= 4 << 5;
        break;
      // reserved: c |= 5 << 5; break;
      // reserved: c |= 6 << 5; break;
      // reserved: c |= 7 << 5; break;
      default:
        throw new Error("Unknown destination of set()");
    }
    c |= val;
    this.code.push(c);
    return this;
  }

  nop() {
    return this.mov("y", "y");
  }

  // additional
  label(name) {
    this.labels[name] = this.code.length;
    return this;
  }

  wrap_target() {
    return this.label("wrap_target");
  }

  wrap() {
    return this.label("wrap");
  }

  side(val) {
    const i = this.code.length - 1;
    let c = this.code[i];
    c |= val << (13 - this.sideset);
    this.code[i] = c;
    return this;
  }

  delay(val) {
    const i = this.code.length - 1;
    let c = this.code[i];
    c |= val << 8;
    this.code[i] = c;
    return this;
  }

  _updateJmps() {
    this.jmps.forEach((jmp) => {
      let c = this.code[jmp.offset];
      c |= this.labels[jmp.target];
      this.code[jmp.offset] = c;
    });
    this.jmps = [];
  }

  toBinary() {
    this._updateJmps();
    return new Uint16Array(this.code);
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
}

class StateMachine {
  constructor(id, asm, options) {
    this.pio = id > 3 ? 1 : 0;
    this.sm = id - this.pio * 4;
    if (!asm._pio) asm._pio = new Array(2);
    if (!asm._pio[this.pio]) {
      const bin = asm.toBinary();
      this.offset = pio_native.pio_add_program(this.pio, bin);
      // this.offset = 0; // TODO: THIS IS FOR TEST
      this.length = bin.length;
      asm._pio[this.pio] = {
        offset: this.offset,
      };
    }
    pio_native.pio_sm_init(
      this.pio,
      this.sm,
      Object.assign(
        {
          // freq,
          inBase: 0,
          outBase: 0,
          outCount: 32,
          setBase: 0,
          setCount: 0,
          sidesetBase: 0,
          sideset: asm.sideset > 0,
          sidesetBitCount: asm.sideset,
          sidesetOpt: false,
          sidesetPindirs: false,
          jmpPin: 0,
          wrapTarget: this.offset + (asm.labels["wrap_target"] ?? 0),
          wrap: this.offset + (asm.labels["wrap"] ?? asm.code.length - 1),
          inShiftDir: PIO.SHIFT_RIGHT,
          autopush: false,
          pushThreshold: 32,
          outShiftDir: PIO.SHIFT_RIGHT,
          autopull: false,
          pullThreshold: 32,
          fifoJoin: PIO.FIFO_JOIN_NONE,
          outSticky: false,
          outEnablePin: -1,
          movStatusSel: PIO.TX_LESSTHAN,
          movStatusN: 0,
        },
        options
      )
    );
  }

  active(value) {
    pio_native.pio_sm_set_enabled(this.pio, this.sm, value);
  }

  restart() {}

  exec(inst) {}

  get() {
    return pio_native.pio_sm_get(this.pio, this.sm);
  }

  put(value) {
    pio_native.pio_sm_put(this.pio, this.sm, value);
  }

  irq() {}
}

exports.PIO = PIO;
exports.ASM = ASM;
exports.StateMachine = StateMachine;

/*
const {ASM, StateMachine} = require('pio');
const asm = new ASM();
asm
  .push(...)
  .set(...)
  .jmp(...)
  ...

const sm = new StateMachine(0, asm, options);
sm.put(1);
sm.put(2);
sm.get();
const inst = (new ASM()).set('pins', 1).toBinary()[0];
sm.exec(inst);
...
*/

/* --------------------- examples ------------------------- */

/*
function print(asm) {
  console.log(
    `[${asm
      .toBinary()
      .map((c) => c.toString(16))
      .join(", ")}]`
  );
}

// hello.pio
const hello_asm = new ASM();
hello_asm.label("loop").pull().out("pins", 1).jmp("loop");
print(hello_asm);

// addition.pio
const addition_asm = new ASM();
addition_asm
  .pull()
  .mov("x", "~osr")
  .pull()
  .mov("y", "osr")
  .jmp("test")
  .label("incr")
  .jmp("x--", "test")
  .label("test")
  .jmp("y--", "incr")
  .mov("isr", "~x")
  .push();
print(addition_asm);

// clocked-input.pio
const clocked_input_asm = new ASM();
clocked_input_asm.wait(0, "pin", 1).wait(1, "pin", 1).in("pins", 1);
print(clocked_input_asm);

// squareware_fast.pio
const squareware_fast_asm = new ASM();
squareware_fast_asm
  .set("pindirs", 1)
  .wrap_target()
  .set("pins", 1)
  .set("pins", 0)
  .wrap();
print(squareware_fast_asm);
console.log(squareware_fast_asm);
*/
