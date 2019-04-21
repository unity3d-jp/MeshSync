#include "pch.h"
#include "msblenContext.h"

static bool msblenExport(msblenContext& self, msblenContext::SendTarget target, msblenContext::SendScope scope)
{
    if (!self.isServerAvailable()) {
        self.logInfo("MeshSync: Server not available. %s", self.getErrorMessage().c_str());
        return false;
    }

    if (target == msblenContext::SendTarget::Objects) {
        self.wait();
        self.sendObjects(msblenContext::SendScope::All, true);
    }
    else if (target == msblenContext::SendTarget::Materials) {
        self.wait();
        self.sendMaterials(true);
    }
    else if (target == msblenContext::SendTarget::Animations) {
        self.wait();
        self.sendAnimations(msblenContext::SendScope::All);
    }
    else if (target == msblenContext::SendTarget::Everything) {
        self.wait();
        self.sendMaterials(true);
        self.wait();
        self.sendObjects(msblenContext::SendScope::All, true);
        self.wait();
        self.sendAnimations(msblenContext::SendScope::All);
    }
    return true;
}

PYBIND11_PLUGIN(MeshSyncClientBlender)
{
    py::module mod("MeshSyncClientBlender", "Python bindings for MeshSync");

#define BindMethod(Name) .def(#Name, &self_t::Name)
#define BindMethodF(Name, ...) .def(#Name, __VA_ARGS__)
#define BindProperty(Name, ...) .def_property(#Name, __VA_ARGS__)
    {
        using self_t = msblenContext;
        py::class_<msblenContext, msbContextPtr>(mod, "Context")
            .def(py::init<>())
            .def_property_readonly("PLUGIN_VERSION", [](const msblenContext& self) { return std::string(msPluginVersionStr); })
            .def_property_readonly("PROTOCOL_VERSION", [](const msblenContext& self) { return std::to_string(msProtocolVersion); })
            .def_property_readonly("TARGET_OBJECTS",    [](const msblenContext& self) { return (int)msblenContext::SendTarget::Objects; })
            .def_property_readonly("TARGET_MATERIALS",  [](const msblenContext& self) { return (int)msblenContext::SendTarget::Materials; })
            .def_property_readonly("TARGET_ANIMATIONS", [](const msblenContext& self) { return (int)msblenContext::SendTarget::Animations; })
            .def_property_readonly("TARGET_EVERYTHING", [](const msblenContext& self) { return (int)msblenContext::SendTarget::Everything; })
            .def_property_readonly("SCOPE_NONE",        [](const msblenContext& self) { return (int)msblenContext::SendScope::None; })
            .def_property_readonly("SCOPE_ALL",         [](const msblenContext& self) { return (int)msblenContext::SendScope::All; })
            .def_property_readonly("SCOPE_UPDATED",     [](const msblenContext& self) { return (int)msblenContext::SendScope::Updated; })
            .def_property_readonly("SCOPE_SELECTED",    [](const msblenContext& self) { return (int)msblenContext::SendScope::Selected; })
            .def_property_readonly("is_server_available", [](msblenContext& self) { return self.isServerAvailable(); })
            .def_property_readonly("error_message", [](msblenContext& self) { return self.getErrorMessage(); })
            BindMethod(flushPendingList)
            BindMethodF(setup, [](msblenContext& self, py::object ctx) { bl::setup(ctx); })
            BindMethodF(clear, [](msblenContext& self, py::object ctx) { self.clear(); })
            BindMethodF(exportUpdatedObjects, [](msblenContext& self) {
                self.sendObjects(msblenContext::SendScope::Updated, false);
            })
            BindMethodF(export, [](msblenContext& self, int _target) {
                msblenExport(self, (msblenContext::SendTarget)_target, msblenContext::SendScope::All);
            })
            BindProperty(server_address,
                [](const msblenContext& self) { return self.getSettings().client_settings.server; },
                [](msblenContext& self, const std::string& v) { self.getSettings().client_settings.server = v; })
            BindProperty(server_port,
                [](const msblenContext& self) { return self.getSettings().client_settings.port; },
                [](msblenContext& self, uint16_t v) { self.getSettings().client_settings.port = v; })
            BindProperty(scene_name,
                [](const msblenContext& self) { return self.getSettings().scene_settings.name; },
                [](msblenContext& self, const std::string& v) { self.getSettings().scene_settings.name = v; })
            BindProperty(scale_factor,
                [](const msblenContext& self) { return self.getSettings().scene_settings.scale_factor; },
                [](msblenContext& self, float v) { self.getSettings().scene_settings.scale_factor = v; })
            BindProperty(handedness,
                [](const msblenContext& self) { return (int)self.getSettings().scene_settings.handedness; },
                [](msblenContext& self, int v) { (int&)self.getSettings().scene_settings.handedness = v; })
            BindProperty(sync_meshes,
                [](const msblenContext& self) { return (int)self.getSettings().sync_meshes; },
                [](msblenContext& self, int v) { (int&)self.getSettings().sync_meshes = v; })
            BindProperty(sync_normals,
                [](const msblenContext& self) { return (int)self.getSettings().sync_normals; },
                [](msblenContext& self, bool v) { (int&)self.getSettings().sync_normals = v; })
            BindProperty(sync_uvs,
                [](const msblenContext& self) { return self.getSettings().sync_uvs; },
                [](msblenContext& self, bool v) { self.getSettings().sync_uvs = v; })
            BindProperty(sync_colors,
                [](const msblenContext& self) { return self.getSettings().sync_colors; },
                [](msblenContext& self, bool v) { self.getSettings().sync_colors = v; })
            BindProperty(make_double_sided,
                [](const msblenContext& self) { return self.getSettings().make_double_sided; },
                [](msblenContext& self, bool v) { self.getSettings().make_double_sided = v; })
            BindProperty(bake_modifiers,
                [](const msblenContext& self) { return self.getSettings().bake_modifiers; },
                [](msblenContext& self, bool v) { self.getSettings().bake_modifiers = v; })
            BindProperty(convert_to_mesh,
                [](const msblenContext& self) { return self.getSettings().convert_to_mesh; },
                [](msblenContext& self, bool v) { self.getSettings().convert_to_mesh = v; })
            BindProperty(sync_bones,
                [](const msblenContext& self) { return self.getSettings().sync_bones; },
                [](msblenContext& self, bool v) { self.getSettings().sync_bones = v; })
            BindProperty(sync_blendshapes,
                [](const msblenContext& self) { return self.getSettings().sync_blendshapes; },
                [](msblenContext& self, bool v) { self.getSettings().sync_blendshapes = v; })
            BindProperty(sync_textures,
                [](const msblenContext& self) { return self.getSettings().sync_textures; },
                [](msblenContext& self, bool v) { self.getSettings().sync_textures = v; })
            BindProperty(sync_cameras,
                [](const msblenContext& self) { return self.getSettings().sync_cameras; },
                [](msblenContext& self, bool v) { self.getSettings().sync_cameras = v; })
            BindProperty(sync_lights,
                [](const msblenContext& self) { return self.getSettings().sync_lights; },
                [](msblenContext& self, bool v) { self.getSettings().sync_lights = v; })
            BindProperty(animation_ts,
                [](const msblenContext& self) { return self.getSettings().animation_timescale; },
                [](msblenContext& self, float v) { self.getSettings().animation_timescale = v; })
            BindProperty(animation_interval,
                [](const msblenContext& self) { return self.getSettings().animation_frame_interval; },
                [](msblenContext& self, int v) { self.getSettings().animation_frame_interval = v; })
            BindProperty(keyframe_reduction,
                [](const msblenContext& self) { return self.getSettings().keyframe_reduction; },
                [](msblenContext& self, int v) { self.getSettings().keyframe_reduction = v; })
            BindProperty(keep_flat_curves,
                [](const msblenContext& self) { return self.getSettings().keep_flat_curves; },
                [](msblenContext& self, int v) { self.getSettings().keep_flat_curves = v; })
            BindProperty(multithreaded,
                [](const msblenContext& self) { return self.getSettings().multithreaded; },
                [](msblenContext& self, int v) { self.getSettings().multithreaded = v; })
                ;
    }
#undef BindMethod
#undef BindMethodF
#undef BindProperty

    return mod.ptr();
}
