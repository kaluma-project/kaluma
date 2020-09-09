
function xinspect(o,i){
  if(typeof i=='undefined')i='';
  if(i.length>50)return '[MAX ITERATIONS]';
  var r=[];
  for(var p in o){
      var t=typeof o[p];
      r.push(i+'"'+p+'" ('+t+') => '+(t=='object' ? 'object:'+xinspect(o[p],i+'  ') : o[p]+''));
  }
  return r.join(i+'\n');
}
