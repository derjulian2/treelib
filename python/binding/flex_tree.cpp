
#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(treelib, m, py::mod_gil_not_used())
{
    m.doc() = "treelib python-binding. created using pybind11.";
    m.def("add", &add, "a function that adds two numbers");
}
