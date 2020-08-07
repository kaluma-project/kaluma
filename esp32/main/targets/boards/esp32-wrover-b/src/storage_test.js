print("in script ok\n");
var lib=require('storage');
var storage = new lib.Storage();
storage.setItem('value1', 'abcd');
storage.setItem('value2', '1234');
var val1 = storage.getItem('value1');
var val2 = storage.getItem('value2');
console.log('val1 : '+val1);
console.log('val2 : '+val2);