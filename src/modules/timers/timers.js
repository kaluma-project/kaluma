
var timers = process.binding(process.binding.timers);

function setTimeout(callback, timeout) {
  return timers.setTimer(callback, timeout, false);
}

function setInterval(callback, interval) {
  return timers.setTimer(callback, interval, true);
}

function clearTimeout(timerId) {
  return timers.clearTimer(timerId);
}

function clearInterval(timerId) {
  return timers.clearTimer(timerId);
}

exports.setTimeout = setTimeout;
exports.setInterval = setInterval;
exports.clearTimeout = clearTimeout;
exports.clearInterval = clearInterval;
