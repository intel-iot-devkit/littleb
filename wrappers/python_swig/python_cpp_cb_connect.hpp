#ifdef PYTHON2
    #include <python2.7/Python.h>
#elif PYTHON3
    #include <python3.5/Python.h>
#endif

/**
 * callback function for charactersitic read event middle man function for python use
 */
static int cbReadEvent(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    std::vector<uint8_t> res = parseUartServiceMessage(message);
    PyObject* result = PyList_New(0);
    for (unsigned int i = 0; i < res.size(); ++i)
    {
        #ifdef PYTHON2
            PyList_Append(result, PyInt_FromSize_t(unsigned(res[i])));
        #elif PYTHON3
            PyList_Append(result, Py_BuildValue("i", unsigned(res[i])));
        #endif
        
        
    }
    int (*cb_python) (PyObject*, PyObject*) = (int (*) (PyObject*, PyObject*)) userdata;
    cb_python( result,  result);
    return 0;
}
/**
 * callback function for change_state middle man function for python use
 */
int cbChangeState(lb_bl_property_change_notification bpcn, void* userdata) {
    #ifdef PYTHON2
        PyIntObject* result = (PyIntObject*) PyInt_FromLong(bpcn);
        int (*cb_python) (PyIntObject*) = (int (*) (PyIntObject*)) userdata;
        cb_python(result);
    #elif PYTHON3
        PyObject* result = (PyObject*) Py_BuildValue("i", bpcn);
        int (*cb_python) (PyObject*) = (int (*) (PyObject*)) userdata;
        cb_python(result);
    #endif
    return 0;	
}
