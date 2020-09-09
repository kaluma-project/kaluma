console.log('net_test 0.1');

function xinspect(o,i){
    if(typeof i=='undefined')i='';
    if(i.length>50) return '[MAX ITERATIONS]';
    var r=[];
    for(var p in o){
	var t=typeof o[p];
	r.push(i+'\''+p+'\' ('+t+') => '+(t=='object' ? 'object:'+xinspect(o[p],i+'  ') : o[p]+''));
    }
    return r.join(i+'\n');
} 

var WIFI = require('wifi').WiFi;

var wifi = new WIFI();

wifi.assoc_cb = function() {
    
    console.log('wifi associated');
    
    var net = require('net');
    var conn = { host: '115.41.222.126', port: 80 }; // enter valid host ip
    var client = net.createConnection(conn, () => {
      // 'connect' listener.
      console.log('connected to server!');
      client.write('GET / HTTP/1.1\r\n\r\n');
    });
    client.on('data', (data) => {
      console.log(data);
      //client.end();
    });
    client.on('end', () => {
      console.log('disconnected from server');
    });
}

wifi.connect({
	ssid:'',
	password:''
}, function(err) {
	console.log('connect ret:' + err);
});

