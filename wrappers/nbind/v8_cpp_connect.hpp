#include "uv.h"
#include <v8.h>
#include <sstream>
#include <algorithm>
#include <iterator>
using namespace v8;

class callbackClass
{
	public:
		int index;
		void* owner;
};

class readEvent: public callbackClass
{
	public:
    std::vector<uint8_t> msg;
        // int error;
        //todo pass errors
		sd_bus_error* error;
};

class stateChangedEvent: public callbackClass
{
	public:
		lb_bl_property_change_notification bpcn;
};



static void beforeFunc(uv_work_t* a){

}
static void state_change_run(uv_work_t* a, int status) {

    callbackClass* cs = (callbackClass*) a->data;
    Device* e = (Device*) cs->owner;
    
    if(!e->r_call.IsEmpty()) {
        Nan::HandleScope scope;
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, e->r_call);
        
        if (!func.IsEmpty()) { 
            stateChangedEvent* sce = (stateChangedEvent*)cs;
            
            const unsigned argc = 2;
            v8::Local<v8::Value> argv[argc]= { Null(isolate), v8::Integer::New(isolate, (int)sce->bpcn)};
            
           func->Call(v8::Null(isolate), argc, argv);
          
        }
    }
    else {
        std::cout<<"empty r_call"<<std::endl;
    }
}

static void read_event_run(uv_work_t* a, int status) {
  
    callbackClass* cs = (callbackClass*) a->data;

    Device* e = (Device*) cs->owner;
 
    if(!e->r_call_event.IsEmpty()) {
        Nan::HandleScope scope;
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, e->r_call_event);
        
        if (!func.IsEmpty()) { 
			readEvent* re = (readEvent*)cs;  
			const unsigned argc = 2;

            // std::cout<<"in read_event_run copying vector: "<<std::endl;
            v8::Handle<v8::Array> result = v8::Array::New(isolate, re->msg.size());
            for (int i = 0; i < re->msg.size(); i++) {
                (void)result->Set(i, v8::Number::New(isolate, re->msg[i]));
                
            }

    		v8::Local<v8::Value> argv[argc] = { Null(isolate),  result}; //TODO    
		    func->Call(v8::Null(isolate) , argc, argv);
        }
    }
    else {
        std::cout<<"empty r_call"<<std::endl;
    }
}
static int uvworkReadEvent(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    

    uv_work_t* req = new uv_work_t;
    readEvent* st = new readEvent();
    st->index =1; 
    st->owner = userdata;
    st->error = error;
    st->msg = parseUartServiceMessage(message);
    // std::cout<<"finished parsing"<<std::endl;
    req->data = (void*) st;
    // req->data = userdata;
    uv_queue_work(uv_default_loop(), req, beforeFunc, read_event_run);
    return 0;
}
int uvworkStateChange(lb_bl_property_change_notification bpcn, void* userdata) {
    std::cout<<"state change uvwork"<<std::endl;
	uv_work_t* req = new uv_work_t;
	stateChangedEvent* st = new stateChangedEvent();
	st->index =2; st->bpcn = bpcn; st->owner = userdata;
    req->data = (void*) st;
    uv_queue_work(uv_default_loop(), req, beforeFunc, state_change_run);
    return 0;	
}

    