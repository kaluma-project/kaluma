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
  
var http = require('http');

var WIFI = require('wifi').WiFi;
var wifi = new WIFI();
print('wifi == ' + xinspect(wifi) + '\n');

wifi.assoc_cb = function() { 
	print('assoc_cb\n');
	http.get({
		host: '192.168.17.103',
		port: 3000,
		path: '/'
	}, function(resp) {
		var str = '';
		resp.on('data', function(chunk) {
			console.log('chunk: ' + chunk);
			str += chunk;
		});
		resp.on('data', function(chunk) {
			console.log('resp.end : ' + str);
		});
	});	
};

wifi.connect_cb = function() { 
	print('connect_cb\n');
};

wifi.disconnect_cb = function() { 
	print('disconnect_cb\n');
};

wifi.connect({
	ssid:'',
	password:''
}, function(err) {
	print('connect ret:' + err);
});
