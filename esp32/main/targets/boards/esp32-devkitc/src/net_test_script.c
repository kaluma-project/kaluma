const char* const net_test_script =
"console.log('net_test 0.1');\n\nfunction xinspect(o,i){\nif(typeof i=='undefined')i='';\nif(i.length>50) return '[MAX ITERATIONS]';\nvar r=[];\nfor(var p in o){\nvar t=typeof o[p];\nr.push(i+'\\''+p+'\\' ('+t+') => '+(t=='object' ? 'object:'+xinspect(o[p],i+'  ') : o[p]+''));\n}\nreturn r.join(i+'\\n');\n}\n\nvar WIFI = require('wifi').WiFi;\n\nvar wifi = new WIFI();\n\nwifi.assoc_cb = function() {\n\nconsole.log('wifi associated');\n\nvar net = require('net');\nvar conn = { host: '115.41.222.126', port: 80 }; // enter valid host ip\nvar client = net.createConnection(conn, () => {\n// 'connect' listener.\nconsole.log('connected to server!');\nclient.write('GET / HTTP/1.1\\r\\n\\r\\n');\n});\nclient.on('data', (data) => {\nconsole.log(data);\n//client.end();\n});\nclient.on('end', () => {\nconsole.log('disconnected from server');\n});\n}\n\nwifi.connect({\nssid:'',\npassword:''\n}, function(err) {\nconsole.log('connect ret:' + err);\n});\n\n";