#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_types(py::module_& m);
void init_document(py::module_& m);
void init_snapshot(py::module_& m);

PYBIND11_MODULE(_sanyi_core, m)
{
    m.doc() = "SanYi CAD PyBindCore facade extension";

    m.def("version", []() {
        return "0.1.0";
    });

    init_types(m);
    init_document(m);
    init_snapshot(m);
}