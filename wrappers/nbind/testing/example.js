var nbind = require('nbind');
var lib = nbind.init().lib;

function cb(isolate, my_str){
	console.log("in JS callback func: "+ my_str);		
}
var a = new lib.ClassExample();
a.RunCallback(cb);
a.foo(2);

// a.register_func(callback);
var b =  a.foo(21);
console.log("b: "+ b);
// var c = new lib.ClassExample("Don't panic");
// var cb = new lib.Callbacks();
// cb.register_state_call(callback);
// console.log(cb.test);
// cb.moshe(2);



