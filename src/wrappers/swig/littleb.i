/* File: littleb.i */
%module(naturalvar=1) littleb
%define BUILDING_WITH_SWIG
%enddef
%include std_vector.i
%include std_string.i
%include "stdint.i"
%include exception.i
%include <std_shared_ptr.i>
%shared_ptr(BleCharactersitic)
%shared_ptr(BleService)
%shared_ptr(Device)

%typemap(in) void* {
    $1 = (void*) PyInt_AsLong($input);
}

%{
#define BUILDING_WITH_SWIG

#define SWIG_FILE_WITH_INIT
#include "device.h"
#include "devicemanager.h"
#include "littlebtypes.h"

%}

%exception {
    try {
        $action
    } catch(const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch(...) {
        SWIG_exception(SWIG_ValueError, "Unknown exception");
    }
}

%include "littlebtypes.h"
%include "device.h"
%include "devicemanager.h"

%template(uintVector) std::vector<uint8_t>;
%template(intVector) std::vector<int>;
%template(BleServicePtrVec) std::vector<std::shared_ptr<BleService> >;
%template(BleCharactersiticPtrVec) std::vector<std::shared_ptr<BleCharactersitic> >;


%pythoncode
%{

import ctypes

# a ctypes callback prototype



def registerCallbackEventRead(dev, uuid, py_callback_ev):
    py_callback_type_re = ctypes.CFUNCTYPE(ctypes.c_int,  ctypes.py_object, ctypes.py_object)
    # wrap the python callback with a ctypes function pointer
    func_er = py_callback_type_re(py_callback_ev)

    # get the function pointer of the ctypes wrapper by casting it to void* and taking its value
    func_er_ptr = ctypes.cast(func_er, ctypes.c_void_p).value
    dev.registerCallbackReadEvent(uuid, func_er_ptr)

def registerCallbackChangeState(dev, py_callback_cs):
    py_callback_type_cs = ctypes.CFUNCTYPE(ctypes.c_int,  ctypes.py_object)
    f_cs = py_callback_type_cs(py_callback_cs)
    f_cs_ptr = ctypes.cast(f_cs, ctypes.c_void_p).value
    dev.registerCallbackStateEvent(f_cs_ptr)
%}





