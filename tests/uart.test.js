const { Gpio } = require( 'onoff' );
const { UART } = require('./uart')
const util = require('./util')
const rpi = require('./rpi')

describe('UART', () => {
  beforeAll(async () => {
    await rpi.tty.open()
    await rpi.tty.write('\r\r')
    await rpi.tty.write('.reset\r')
    await rpi.tty.write('.echo off\r')
    await rpi.serial0.open()
    await rpi.tty.write('var UART = require("uart").UART\r')
    await rpi.tty.write('var uart0 = new UART(0, {baudrate: 115200})\r')
  });

  afterAll(async () => {
    await rpi.tty.write('\r.echo on\r')
    await rpi.serial0.close()
    await rpi.tty.close()
  });

  test('uart-0-write', async () => {
    const data = 'hello, world!'
    rpi.serial0.clear()
    await rpi.tty.write(`uart0.write('${data}');\r`)
    await util.wait(100)
    const val1 = await rpi.serial0.read()
    expect(val1).toBe(data)
  })

  test('uart-0-read', async () => {
    const data = 'hello, world!'
    await rpi.tty.write(`var data = ''\r`);
    await rpi.tty.write(`uart0.on('data', (s) => { data += String.fromCharCode.apply(null, s) });\r`)
    await rpi.serial0.write(data)
    await util.wait(100)
    await rpi.tty.write(`uart0.write(data);\r`)
    await util.wait(100)
    const val1 = await rpi.serial0.read()
    expect(val1).toBe(data)
  })
})
