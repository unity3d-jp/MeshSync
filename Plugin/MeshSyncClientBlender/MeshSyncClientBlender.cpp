#include "pch.h"
#include "MeshSyncClientBlender.h"
using namespace mu;


PYBIND11_PLUGIN(MeshSyncClientBlender)
{
    py::module mod("MeshSyncClientBlender", "Python bindings for MeshSync");

#define BindMethod(Name) .def(#Name, &self_t::Name)
#define BindMethod2(Name, ...) .def(#Name, __VA_ARGS__)
#define BindField(Name) .def_readwrite(#Name, &self_t::Name)
#define BindProperty(Name, ...) .def_property(#Name, __VA_ARGS__)
    {
        using self_t = msbContext;
        py::class_<msbContext, msbContextPtr>(mod, "Context")
            .def(py::init<>())
            BindMethod(setup)
            BindMethod(isSending)
            BindMethod(flushPendingList)
            BindMethod(prepare)
            BindMethod(syncAll)
            BindMethod(syncUpdated)
            BindMethod(send)
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
                [](msbContext& self, int v) { (int&)self.getSettings().sync_normals = v; })
            BindProperty(sync_uvs,
                [](const msbContext& self) { return self.getSettings().sync_uvs; },
                [](msbContext& self, bool v) { self.getSettings().sync_uvs = v; })
            BindProperty(sync_colors,
                [](const msbContext& self) { return self.getSettings().sync_colors; },
                [](msbContext& self, bool v) { self.getSettings().sync_colors = v; })
            BindProperty(sync_bones,
                [](const msbContext& self) { return self.getSettings().sync_bones; },
                [](msbContext& self, bool v) { self.getSettings().sync_bones = v; })
            BindProperty(sync_poses,
                [](const msbContext& self) { return self.getSettings().sync_poses; },
                [](msbContext& self, bool v) { self.getSettings().sync_poses = v; })
            BindProperty(sync_blendshapes,
                [](const msbContext& self) { return self.getSettings().sync_blendshapes; },
                [](msbContext& self, bool v) { self.getSettings().sync_blendshapes = v; })
            BindProperty(sync_animations,
                [](const msbContext& self) { return self.getSettings().sync_animations; },
                [](msbContext& self, bool v) { self.getSettings().sync_animations = v; })
            BindProperty(sync_cameras,
                [](const msbContext& self) { return self.getSettings().sync_cameras; },
                [](msbContext& self, bool v) { self.getSettings().sync_cameras = v; })
            BindProperty(sync_lights,
                [](const msbContext& self) { return self.getSettings().sync_lights; },
                [](msbContext& self, bool v) { self.getSettings().sync_lights = v; })
            BindProperty(sample_animation,
                [](const msbContext& self) { return self.getSettings().sample_animation; },
                [](msbContext& self, bool v) { self.getSettings().sample_animation = v; })
            BindProperty(animation_sps,
                [](const msbContext& self) { return self.getSettings().animation_sps; },
                [](msbContext& self, int v) { self.getSettings().animation_sps = v; })
            ;
    }
#undef BindMethod
#undef BindMethod2
#undef BindField
#undef BindProperty

    return mod.ptr();
}
