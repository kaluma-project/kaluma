const http = require('http');
http.get({
  host : '192.168.17.103',
  port : 8000,
  path : '/index.js'
}, (res) => {
  res.resume();
  res.on('data', (chunk) => {
    console.log(`Chunk : ${chunk}`);
  });
  res.on('end', () => {
    console.log('Done.');
  });
})
