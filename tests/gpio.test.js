const { Gpio } = require( 'onoff' );
const { UART } = require('./uart')
const util = require('./util')
const rpi = require('./rpi')

describe('GPIO', () => {
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

  test('gpio-0-read', async () => {
    await rpi.tty.write('pinMode(0, OUTPUT)\r')
    await rpi.tty.write('digitalWrite(0, LOW)\r')
    await util.wait(10)
    const val1 = rpi.gpioRead(4)
    expect(val1).toBe(0)
    await rpi.tty.write('digitalWrite(0, HIGH)\r')
    await util.wait(10)
    const val2 = rpi.gpioRead(4)
    expect(val2).toBe(1)
  })

  test('gpio-0-write', async () => {
    await rpi.tty.write('pinMode(0, INPUT)\r')
    rpi.serial0.clear()
    rpi.gpioWrite(4, 0) // Set LOW
    await rpi.tty.write('uart0.write(digitalRead(0).toString());\r')
    const val1 = await rpi.serial0.read()
    expect(val1).toBe('0')
    rpi.gpioWrite(4, 1) // Set HIGH
    await rpi.tty.write('uart0.write(digitalRead(0).toString());\r')
    const val2 = await rpi.serial0.read()
    expect(val2).toBe('1')
  })
})
