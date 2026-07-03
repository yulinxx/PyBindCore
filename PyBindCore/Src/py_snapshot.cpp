#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "PyFacade/FacadeTypes.h"

namespace py = pybind11;

void init_snapshot(py::module_& m)
{
    py::class_<PyFacade::EntitySnapshot>(m, "EntitySnapshot")
        .def(py::init<>())
        .def_readwrite("id", &PyFacade::EntitySnapshot::id)
        .def_readwrite("type", &PyFacade::EntitySnapshot::type)
        .def_readwrite("base_point", &PyFacade::EntitySnapshot::basePoint)
        .def_readwrite("points", &PyFacade::EntitySnapshot::points)
        .def_readwrite("radius", &PyFacade::EntitySnapshot::radius)
        .def_readwrite("closed", &PyFacade::EntitySnapshot::closed);

    py::class_<PyFacade::SceneSnapshot>(m, "SceneSnapshot")
        .def(py::init<>())
        .def_readwrite("entities", &PyFacade::SceneSnapshot::entities)
        .def_readwrite("bounds", &PyFacade::SceneSnapshot::bounds)
        .def_readwrite("entity_count", &PyFacade::SceneSnapshot::entityCount);

    py::class_<PyFacade::ApplyChanges>(m, "ApplyChanges")
        .def(py::init<>())
        .def_readwrite("add", &PyFacade::ApplyChanges::add)
        .def_readwrite("remove", &PyFacade::ApplyChanges::remove);
}