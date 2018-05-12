#include "pch.h"
#include "MeshSyncClientBlender.h"
using namespace mu;


PYBIND11_PLUGIN(MeshSyncClientBlender)
{
    py::module mod("MeshSyncClientBlender", "Python bindings for MeshSync");

#define BindMethod(Name) .def(#Name, &self_t::Name)
#define BindMethod2(Name, ...) .def(#Name, __VA_ARGS__)
#define BindProperty(Name, ...) .def_property(#Name, __VA_ARGS__)
    {
        using self_t = msbContext;
        py::class_<msbContext, msbContextPtr>(mod, "Context")
            .def(py::init<>())
            BindMethod(flushPendingList)
            BindMethod2(setup, [](msbContext& self, py::object ctx) { bl::setup(ctx); })
            BindMethod2(sendSceneAll, [](msbContext& self) { self.sendScene(msbContext::SendScope::All); })
            BindMethod2(sendSceneUpdated, [](msbContext& self) { self.sendScene(msbContext::SendScope::Updated); })
            BindMethod2(sendSceneSelected, [](msbContext& self) { self.sendScene(msbContext::SendScope::Selected); })
            BindMethod2(sendAnimationsAll, [](msbContext& self) { self.sendAnimations(msbContext::SendScope::All); })
            BindMethod2(sendAnimationsSelected, [](msbContext& self) { self.sendAnimations(msbContext::SendScope::Selected); })
            BindProperty(server_address,
                [](const msbContext& self) { return self.getSettings().client_settings.server; },
                [](msbContext& self, const std::string& v) { self.getSettings().client_settings.server = v; })
            BindProperty(server_port,
                [](const msbContext& self) { return self.getSettings().client_settings.port; },
                [](msbContext& self, uint16_t v) { self.getSettings().client_settings.port = v; })
            BindProperty(scene_name,
                [](const msbContext& self) { return self.getSettings().scene_settings.name; },
                [](msbContext& self, const std::string& v) { self.getSettings().scene_settings.name = v; })
            BindProperty(scale_factor,
                [](const msbContext& self) { return self.getSettings().scene_settings.scale_factor; },
                [](msbContext& self, float v) { self.getSettings().scene_settings.scale_factor = v; })
            BindProperty(handedness,
                [](const msbContext& self) { return (int)self.getSettings().scene_settings.handedness; },
                [](msbContext& self, int v) { (int&)self.getSettings().scene_settings.handedness = v; })
            BindProperty(sync_meshes,
                [](const msbContext& self) { return (int)self.getSettings().sync_meshes; },
                [](msbContext& self, int v) { (int&)self.getSettings().sync_meshes = v; })
            BindProperty(sync_normals,
                [](const msbContext& self) { return (int)self.getSettings().sync_normals; },
                [](msbContext& self, bool v) { (int&)self.getSettings().sync_normals = v; })
            BindProperty(sync_uvs,
                [](const msbContext& self) { return self.getSettings().sync_uvs; },
                [](msbContext& self, bool v) { self.getSettings().sync_uvs = v; })
            BindProperty(sync_colors,
                [](const msbContext& self) { return self.getSettings().sync_colors; },
                [](msbContext& self, bool v) { self.getSettings().sync_colors = v; })
            BindProperty(sync_bones,
                [](const msbContext& self) { return self.getSettings().sync_bones; },
                [](msbContext& self, bool v) { self.getSettings().sync_bones = v; })
            BindProperty(sync_blendshapes,
                [](const msbContext& self) { return self.getSettings().sync_blendshapes; },
                [](msbContext& self, bool v) { self.getSettings().sync_blendshapes = v; })
            BindProperty(sync_cameras,
                [](const msbContext& self) { return self.getSettings().sync_cameras; },
                [](msbContext& self, bool v) { self.getSettings().sync_cameras = v; })
            BindProperty(sync_lights,
                [](const msbContext& self) { return self.getSettings().sync_lights; },
                [](msbContext& self, bool v) { self.getSettings().sync_lights = v; })
            BindProperty(animation_ts,
                [](const msbContext& self) { return self.getSettings().animation_timescale; },
                [](msbContext& self, float v) { self.getSettings().animation_timescale = v; })
            BindProperty(animation_interval,
                [](const msbContext& self) { return self.getSettings().animation_frame_interval; },
                [](msbContext& self, int v) { self.getSettings().animation_frame_interval = v; })
            ;
    }
#undef BindMethod
#undef BindMethod2
#undef BindProperty

    return mod.ptr();
}
