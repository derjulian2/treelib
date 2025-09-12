

#include <python3.12/Python.h>
#include <python3.12/moduleobject.h>

typedef unsigned long long ull;

ull fibonacci(ull n)
{
    if (n <= 1) 
    { return 1; }
    else 
    { return fibonacci(n - 1) + fibonacci(n - 2); }
}

static PyObject* fib(PyObject* self, PyObject* args)
{
    int n;
    if (!PyArg_ParseTuple(args, "i", &n))
        return NULL;
    return Py_BuildValue("i", fibonacci(n));
}

static PyMethodDef fibMethods[] = {
    {"fib", fib, METH_VARARGS, "Calculate the Fibonacci numbers (in C)."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef fibStruct = 
{
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "fib",
    .m_size = 0,
    .m_methods = fibMethods
};

PyMODINIT_FUNC PyInit_fib(void)
{
    return PyModule_Create(&fibStruct);
}