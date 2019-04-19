#include "pch.h"
#include "msbContext.h"


PYBIND11_PLUGIN(MeshSyncClientBlender)
{
    py::module mod("MeshSyncClientBlender", "Python bindings for MeshSync");

#define BindMethod(Name) .def(#Name, &self_t::Name)
#define BindMethodF(Name, ...) .def(#Name, __VA_ARGS__)
#define BindProperty(Name, ...) .def_property(#Name, __VA_ARGS__)
    {
        using self_t = msbContext;
        py::class_<msbContext, msbContextPtr>(mod, "Context")
            .def(py::init<>())
            .def_property_readonly("VERSION", [](const msbContext& self) { return std::string(msReleaseDateStr); })
            .def_property_readonly("TARGET_OBJECTS",    [](const msbContext& self) { return (int)msbContext::SendTarget::Objects; })
            .def_property_readonly("TARGET_MATERIALS",  [](const msbContext& self) { return (int)msbContext::SendTarget::Materials; })
            .def_property_readonly("TARGET_ANIMATIONS", [](const msbContext& self) { return (int)msbContext::SendTarget::Animations; })
            .def_property_readonly("TARGET_EVERYTHING", [](const msbContext& self) { return (int)msbContext::SendTarget::Everything; })
            .def_property_readonly("SCOPE_NONE",        [](const msbContext& self) { return (int)msbContext::SendScope::None; })
            .def_property_readonly("SCOPE_ALL",         [](const msbContext& self) { return (int)msbContext::SendScope::All; })
            .def_property_readonly("SCOPE_UPDATED",     [](const msbContext& self) { return (int)msbContext::SendScope::Updated; })
            .def_property_readonly("SCOPE_SELECTED",    [](const msbContext& self) { return (int)msbContext::SendScope::Selected; })
            BindMethod(flushPendingList)
            BindMethodF(setup, [](msbContext& self, py::object ctx) { bl::setup(ctx); })
            BindMethodF(clear, [](msbContext& self, py::object ctx) { self.clear(); })
            BindMethodF(exportUpdatedObjects, [](msbContext& self) {
                self.sendObjects(msbContext::SendScope::Updated, false);
            })
            BindMethodF(export, [](msbContext& self, int _target) {
                auto target = (msbContext::SendTarget)_target;
                if (target == msbContext::SendTarget::Objects) {
                    self.wait();
                    self.sendObjects(msbContext::SendScope::All, true);
                }
                else if (target == msbContext::SendTarget::Materials) {
                    self.wait();
                    self.sendMaterials(true);
                }
                else if (target == msbContext::SendTarget::Animations) {
                    self.wait();
                    self.sendAnimations(msbContext::SendScope::All);
                }
                else if (target == msbContext::SendTarget::Everything) {
                    self.wait();
                    self.sendMaterials(true);
                    self.wait();
                    self.sendObjects(msbContext::SendScope::All, true);
                    self.wait();
                    self.sendAnimations(msbContext::SendScope::All);
                }
            })
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
            BindProperty(make_double_sided,
                [](const msbContext& self) { return self.getSettings().make_double_sided; },
                [](msbContext& self, bool v) { self.getSettings().make_double_sided = v; })
            BindProperty(bake_modifiers,
                [](const msbContext& self) { return self.getSettings().bake_modifiers; },
                [](msbContext& self, bool v) { self.getSettings().bake_modifiers = v; })
            BindProperty(convert_to_mesh,
                [](const msbContext& self) { return self.getSettings().convert_to_mesh; },
                [](msbContext& self, bool v) { self.getSettings().convert_to_mesh = v; })
            BindProperty(sync_bones,
                [](const msbContext& self) { return self.getSettings().sync_bones; },
                [](msbContext& self, bool v) { self.getSettings().sync_bones = v; })
            BindProperty(sync_blendshapes,
                [](const msbContext& self) { return self.getSettings().sync_blendshapes; },
                [](msbContext& self, bool v) { self.getSettings().sync_blendshapes = v; })
            BindProperty(sync_textures,
                [](const msbContext& self) { return self.getSettings().sync_textures; },
                [](msbContext& self, bool v) { self.getSettings().sync_textures = v; })
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
            BindProperty(keyframe_reduction,
                [](const msbContext& self) { return self.getSettings().keyframe_reduction; },
                [](msbContext& self, int v) { self.getSettings().keyframe_reduction = v; })
            BindProperty(keep_flat_curves,
                [](const msbContext& self) { return self.getSettings().keep_flat_curves; },
                [](msbContext& self, int v) { self.getSettings().keep_flat_curves = v; })
            BindProperty(multithreaded,
                [](const msbContext& self) { return self.getSettings().multithreaded; },
                [](msbContext& self, int v) { self.getSettings().multithreaded = v; })
                ;
    }
#undef BindMethod
#undef BindMethodF
#undef BindProperty

    return mod.ptr();
}
