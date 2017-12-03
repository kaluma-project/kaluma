
var timers = process.binding(process.binding.timers);

function setTimeout(callback, timeout) {
  return timers.set_timer(callback, timeout, false);
}

function setInterval(callback, interval) {
  return timers.set_timer(callback, interval, true);
}

exports.setTimeout = setTimeout;
exports.setInterval = setInterval;
