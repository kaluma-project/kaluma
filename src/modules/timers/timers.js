
function setTimeout(callback, timeout) {
  print('setTimeout called');
}

function setInterval(callback, interval) {
  print('setInterval called');
}

exports.setTimeout = setTimeout;
exports.setInterval = setInterval;
