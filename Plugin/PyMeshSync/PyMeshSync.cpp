#include "pch.h"
#include "PyMeshSync.h"


namespace py = pybind11;

PYBIND11_PLUGIN(PyMeshSync)
{
    py::module mod("PyMeshSync", "Python bindings for MeshSync");

#define BindMethod(Name) .def(#Name, &self_t::Name)
#define BindField(Name) .def_readwrite(#Name, &self_t::Name)
#define BindProperty(Name, Name2) .def_property(#Name, &self_t::get##Name2, &self_t::set##Name2)
#define BindProperty2(Name, ...) .def_property(#Name, __VA_ARGS__)
    {
        using self_t = ms::Transform;
        py::class_<ms::Transform, ms::TransformPtr>(mod, "Transform")
            BindField(path)
            BindField(visible)
            BindProperty(position, Position)
            BindProperty(rotation, Rotation)
            BindProperty(scale, Scale)
            BindMethod(addTranslationKey)
            BindMethod(addRotationKey)
            BindMethod(addScaleKey)
            ;
    }
    {
        using self_t = ms::Camera;
        py::class_<ms::Camera, ms::CameraPtr, ms::Transform>(mod, "Camera")
            BindField(fov)
            BindField(near_plane)
            BindField(far_plane)
            BindField(vertical_aperture)
            BindField(horizontal_aperture)
            BindField(focal_length)
            BindField(focus_distance)
            BindMethod(addFovKey)
            BindMethod(addNearPlaneKey)
            BindMethod(addFarPlaneKey)
            BindMethod(addVerticalApertureKey)
            BindMethod(addHorizontalApertureKey)
            BindMethod(addFocalLengthKey)
            BindMethod(addFocusDistanceKey)
            ;
    }
    {
        using self_t = ms::Light;
        py::class_<ms::Light, ms::LightPtr, ms::Transform>(mod, "Light")
            BindField(type)
            BindProperty(color, Color)
            BindField(intensity)
            BindField(range)
            BindField(spot_angle)
            BindMethod(addColorKey)
            BindMethod(addIntensityKey)
            BindMethod(addRangeKey)
            BindMethod(addSpotAngleKey)
            ;
    }
    {
        using self_t = ms::Mesh;
        py::class_<ms::Mesh, ms::MeshPtr, ms::Transform>(mod, "Mesh")
            BindMethod(addVertex)
            BindMethod(addNormal)
            BindMethod(addUV)
            BindMethod(addColor)
            BindMethod(addCount)
            BindMethod(addIndex)
            BindMethod(addMaterialID)
            ;
    }
    {
        using self_t = pymsContext;
        py::class_<pymsContext>(mod, "Context")
            BindMethod(addTransform)
            BindMethod(addCamera)
            BindMethod(addLight)
            BindMethod(addMesh)
            BindMethod(isSending)
            BindMethod(send)
            BindProperty2(server_address,
                [](const pymsContext& self) { return self.getSettings().client_settings.server; },
                [](pymsContext& self, const std::string& v) { return self.getSettings().client_settings.server = v; })
            BindProperty2(server_port,
                [](const pymsContext& self) { return self.getSettings().client_settings.port; },
                [](pymsContext& self, uint16_t v) { return self.getSettings().client_settings.port = v; })
            BindProperty2(scale_factor,
                [](const pymsContext& self) { return self.getSettings().scale_factor; },
                [](pymsContext& self, float v) { return self.getSettings().scale_factor = v; })
            BindProperty2(auto_sync,
                [](const pymsContext& self) { return self.getSettings().auto_sync; },
                [](pymsContext& self, bool v) { return self.getSettings().auto_sync = v; })
            BindProperty2(sync_delete,
                [](const pymsContext& self) { return self.getSettings().sync_delete; },
                [](pymsContext& self, bool v) { return self.getSettings().sync_delete = v; })
            BindProperty2(sync_camera,
                [](const pymsContext& self) { return self.getSettings().sync_camera; },
                [](pymsContext& self, bool v) { return self.getSettings().sync_camera = v; })
            BindProperty2(sync_animation,
                [](const pymsContext& self) { return self.getSettings().sync_animation; },
                [](pymsContext& self, bool v) { return self.getSettings().sync_animation = v; })
            ;
    }
#undef BindMethod
#undef BindField
#undef BindProperty
#undef BindProperty2

    return mod.ptr();
}
