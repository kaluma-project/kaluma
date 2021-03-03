const SerialPort = require('serialport')

class UART {
  constructor (path, options) {
    const serialOptions = Object.assign({
      autoOpen: false,
      baudRate: 9600
    }, options)
    this.serial = new SerialPort(path, serialOptions)
    this.buffer = ''
    this.serial.on('data', (chunk) => {
      var s = String.fromCharCode.apply(null, chunk);
      this.buffer += s
    })
  }

  open () {
    return new Promise((resolve, reject) => {
      this.serial.open(err => {
        if (err) {
          console.error(err)
          reject(err)
        } else {
          resolve()
        }
      })
    })
  }

  close () {
    return new Promise((resolve, reject) => {
      this.serial.close(err => {
        if (err) {
          console.error(err)
          reject(err)
        } else {
          resolve()
        }
      })
    })
  }

  write (data) {
    return new Promise((resolve, reject) => {
      this.serial.write(data, err => {
        if (err) {
          reject(err)
        } else {
          resolve()
        }
      })
    })
  }

  clear () {
    this.buffer = ''
  }

  read (msec = 10) {
    return new Promise((resolve, reject) => {
      setTimeout(() => {
        const result = this.buffer
        this.buffer = ''
        resolve(result)
      }, msec)
    })
  }
}

exports.UART = UART
