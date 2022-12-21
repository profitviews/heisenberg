#define PY_SSIZE_T_CLEAN
#include <Python.h>

int main(int argc, char *argv[]) {
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;
    int i;

    if (argc < 3) {
        fprintf(stderr,"Usage: runPythonFunction <Python Module> <Function name> [Arguments]\n");
        return 1;
    }

    // argv[1]: Module name
    // argv[2]: Function name
    // argv[3] - argv[n]: Function arguments

    Py_Initialize();
    // Add the local directory to sys.path
    PyObject *sysmodule = PyImport_ImportModule("sys");
    PyObject *syspath = PyObject_GetAttrString(sysmodule, "path");
    PyList_Append(syspath, PyUnicode_FromString("."));
    
    // Import module
    pName = PyUnicode_DecodeFSDefault(argv[1]);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        // Get function name
        pFunc = PyObject_GetAttrString(pModule, argv[2]);
        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(argc - 3);
            // Parse function arguments
            for (i = 0; i < argc - 3; ++i) {
                pValue = PyLong_FromLong(atoi(argv[i + 3]));
                if (!pValue) {
                    Py_DECREF(pArgs);
                    Py_DECREF(pModule);
                    fprintf(stderr, "Cannot convert argument\n");
                    return 1;
                }
                PyTuple_SetItem(pArgs, i, pValue);
            }
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
                printf("C: Result of function call: %ld\n", PyLong_AsLong(pValue));
                Py_DECREF(pValue);
            }
            else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Function call failed\n");
                return 1;
            }
        }
        else {
            if (PyErr_Occurred()) PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
        return 1;
    }
    if (Py_FinalizeEx() < 0) {
        return 1;
    }
    return 0;
}