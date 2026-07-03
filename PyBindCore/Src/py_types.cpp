#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include "PyFacade/FacadeTypes.h"

namespace py = pybind11;

void init_types(py::module_& m)
{
    py::class_<PyFacade::Vec2>(m, "Vec2")
        .def(py::init<>())
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &PyFacade::Vec2::x)
        .def_readwrite("y", &PyFacade::Vec2::y)
        .def("__repr__", [](const PyFacade::Vec2& v) {
        return py::str("Vec2({:.3f}, {:.3f})").format(v.x, v.y);
            });

    py::class_<PyFacade::BBox2>(m, "BBox2")
        .def(py::init<>())
        .def(py::init<double, double, double, double>(),
            py::arg("min_x"), py::arg("min_y"), py::arg("max_x"), py::arg("max_y"))
        .def_readwrite("min_x", &PyFacade::BBox2::minX)
        .def_readwrite("min_y", &PyFacade::BBox2::minY)
        .def_readwrite("max_x", &PyFacade::BBox2::maxX)
        .def_readwrite("max_y", &PyFacade::BBox2::maxY)
        .def_property_readonly("valid", &PyFacade::BBox2::isValid);

    py::class_<PyFacade::EntityRef>(m, "EntityRef")
        .def(py::init<>())
        .def(py::init<int64_t>(), py::arg("id"))
        .def_readwrite("id", &PyFacade::EntityRef::id)
        .def_property_readonly("valid", &PyFacade::EntityRef::valid)
        .def("__repr__", [](const PyFacade::EntityRef& ref) {
        return py::str("EntityRef(id={})").format(ref.id);
            })
        .def(py::self == py::self)
        .def(py::self != py::self);
}