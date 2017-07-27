#include "pch.h"
#include "PyMeshSync.h"


namespace py = pybind11;

PYBIND11_PLUGIN(PyMeshSync)
{
    py::module mod("PyMeshSync", "Python bindings for MeshSYnc");

    py::class_<pymsContext>(mod, "Context", py::metaclass())
        .def("addMesh", &pymsContext::addMesh)
        .def("send", &pymsContext::send)
        ;

    py::class_<pymsMesh>(mod, "Mesh", py::metaclass())
        .def("setPath", &pymsMesh::setPath)
        .def("addVertex", &pymsMesh::addVertex)
        .def("addNormal", &pymsMesh::addNormal)
        .def("addUV", &pymsMesh::addUV)
        .def("addColor", &pymsMesh::addColor)
        .def("addCount", &pymsMesh::addCount)
        .def("addIndex", &pymsMesh::addIndex)
        .def("addMaterialID", &pymsMesh::addMaterialID)
        ;

    return mod.ptr();
}
