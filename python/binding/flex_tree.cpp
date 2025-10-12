
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include "../../include/treelib/flex_tree.hpp"

namespace py = pybind11;

using flex_tree = trl::flex_tree<py::object, std::allocator<py::object>>;

PYBIND11_MODULE(treelib, m, py::mod_gil_not_used())
{
    m.doc() = "treelib python-binding. created using pybind11.";
    
    py::class_<flex_tree>(m,"FlexTree")
        .def(py::init<>())
        .def("append", &flex_tree::append<trl::depth_first_pre_order>)
        .def("prepend", &flex_tree::prepend<trl::depth_first_pre_order>);
}
