#include "pch.h"
#include "PyMeshSync.h"


namespace py = pybind11;

PYBIND11_PLUGIN(PyMeshSync)
{
    py::module mod("PyMeshSync", "Python bindings for MeshSync");

#define Bind(Name) .def(#Name, &self_t::Name)
    {
        using self_t = ms::Transform;
        py::class_<ms::Transform, ms::TransformPtr>(mod, "Transform", py::metaclass())
            Bind(setPath)
            Bind(setVisible)
            Bind(setPosition)
            Bind(setRotation)
            Bind(setScale)
            ;
    }
    {
        using self_t = ms::Camera;
        py::class_<ms::Camera, ms::CameraPtr, ms::Transform>(mod, "Camera", py::metaclass())
            Bind(setFov)
            Bind(setNearPlane)
            Bind(setFarPlane)
            Bind(setVerticalAperture)
            Bind(setHorizontalAperture)
            Bind(setFocalLength)
            Bind(setFocusDistance)
            ;
    }
    {
        using self_t = ms::Light;
        py::class_<ms::Light, ms::LightPtr, ms::Transform>(mod, "Light", py::metaclass())
            Bind(setLightType)
            Bind(setColor)
            Bind(setIntensity)
            Bind(setRange)
            Bind(setSpotAngle)
            ;
    }
    {
        using self_t = ms::Mesh;
        py::class_<ms::Mesh, ms::MeshPtr, ms::Transform>(mod, "Mesh", py::metaclass())
            Bind(addVertex)
            Bind(addNormal)
            Bind(addUV)
            Bind(addColor)
            Bind(addCount)
            Bind(addIndex)
            Bind(addMaterialID)
            ;
    }
    {
        using self_t = pymsContext;
        py::class_<pymsContext>(mod, "Context", py::metaclass())
            Bind(addTransform)
            Bind(addCamera)
            Bind(addLight)
            Bind(addMesh)
            Bind(isSending)
            Bind(send)
            ;
    }
#undef Bind

    return mod.ptr();
}
