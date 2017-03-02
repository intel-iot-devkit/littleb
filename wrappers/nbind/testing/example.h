#include <iostream>
#include <string>
#include "uv.h"
#include <v8.h>
#include "nbind/api.h"
using namespace v8;



class ClassExample {

public:
  
  ClassExample();
  v8::Persistent<v8::Function> r_call;
  v8::Local<v8::Function> func;

  int foo(int x);

  void RunCallback(nbind::cbFunction& func2) {

        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        func = func2.getJsFunction();

        r_call.Reset(isolate, func);

}
};
static void bef(uv_work_t* a){}
static void run(uv_work_t* a, int sta);

static void
uvwork(void* ctx)
{

    uv_work_t* req = new uv_work_t;
    req->data = ctx;
    uv_queue_work(uv_default_loop(), req, bef, run);
}
