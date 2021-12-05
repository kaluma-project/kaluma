const { UART } = require('./uart')

async function foo() {
  var s0 = new UART('/dev/serial0')
  await s0.open()
  const data = await s0.read(10000)
  console.log(data)
}

foo()

/*
const SerialPort = require('serialport')

const comName = '/dev/serial0'
const serialOptions = { baudRate: 9600, autoOpen: false }
const serial = new SerialPort(comName, serialOptions)

serial.on('data', (chunk) => {
  var s = String.fromCharCode.apply(null, chunk);
  console.log(`len=${chunk.length}`)
  console.log(`data="${s}"`)
})

serial.open((err) => {
  if (err) {
    console.error(err)
  } else {
    console.log('connected')
    setTimeout(() => {
      console.log('timeout')
      serial.write('hello\n', (err) => {
        if (err) {
          console.error(err)
        } else {
          console.log('written')
        }
      })
      serial.drain((err) => {
        if (err) {
          console.error(err)
        } else {
          console.log('drained')
        }
      })
    }, 500)
  }
})
*/
