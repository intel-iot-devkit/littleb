#include <iostream>
#include <thread>
#include "example.h"


  ClassExample::ClassExample() {
    
  }

  int ClassExample::foo(int x)
  {

  	std::cout << "running in foo: " << x << "\n";
    std::thread t1(uvwork, this );
    t1.join(); 
  	
    return x;
  }


void run(uv_work_t* a, int sta) {
  
    ClassExample* e = (ClassExample*) a->data;

    if(!e->r_call.IsEmpty()) {
      
      Nan::HandleScope scope;
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, e->r_call);

        if (!func.IsEmpty()) {
            const unsigned argc = 2;
            v8::Local<v8::Value> argv[argc] =
                { Null(isolate), v8::String::NewFromUtf8(isolate, "hello world") };
            func->Call(v8::Null(isolate), argc, argv);
        }
    }
}
  #include "nbind/nbind.h"

#ifdef NBIND_CLASS
NBIND_CLASS(ClassExample) {
  construct<>();
  method(RunCallback);
  method(foo);
}
#endif
