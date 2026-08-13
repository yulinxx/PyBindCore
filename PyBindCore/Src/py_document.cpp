#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "PyFacade/DocumentFacade.h"

namespace py = pybind11;

void init_document(py::module_& m)
{
    py::class_<PyFacade::DocumentFacade, std::shared_ptr<PyFacade::DocumentFacade>>(m, "Document")
        .def_static("create", &PyFacade::DocumentFacade::create, "Create an empty in-memory document")
        .def_static(
            "open", &PyFacade::DocumentFacade::open, py::arg("path"), "Open a document from file (not implemented yet)")
        .def_property_readonly("valid", &PyFacade::DocumentFacade::isValid)
        .def_property_readonly("path", &PyFacade::DocumentFacade::path)
        .def_property_readonly("last_error", &PyFacade::DocumentFacade::lastError)
        .def("add_point", &PyFacade::DocumentFacade::addPoint, py::arg("point"))
        .def("add_line", &PyFacade::DocumentFacade::addLine, py::arg("start"), py::arg("end"))
        .def("add_circle", &PyFacade::DocumentFacade::addCircle, py::arg("center"), py::arg("radius"))
        .def("remove_entity", &PyFacade::DocumentFacade::removeEntity, py::arg("entity"))
        .def("query_box", &PyFacade::DocumentFacade::queryBox, py::arg("box"), py::arg("contained_only") = false)
        .def("export_snapshot", &PyFacade::DocumentFacade::exportSnapshot)
        .def("apply_changes", &PyFacade::DocumentFacade::applyChanges, py::arg("changes"))
        .def("entity_count", &PyFacade::DocumentFacade::entityCount)
        .def("get_entity", &PyFacade::DocumentFacade::getEntity, py::arg("entity"))
        .def("save", &PyFacade::DocumentFacade::save, py::arg("path"))
        .def("close", &PyFacade::DocumentFacade::close);
}