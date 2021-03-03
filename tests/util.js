exports.wait = function (msec = 10) {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve()
    }, msec)
  })
}

exports.waitNextTick = function () {
  return new Promise((resolve, reject) => {
    process.nextTick(() => {
      resolve()
    })
  })
}
