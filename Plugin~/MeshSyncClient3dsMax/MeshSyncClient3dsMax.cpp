#include "pch.h"
#include "msmaxContext.h"

HINSTANCE g_msmax_hinstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_msmax_hinstance = hinstDLL;
        MaxSDK::Util::UseLanguagePackLocale();
        msmaxGetContext(); // initialize instance
        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    return(TRUE);
}

msmaxAPI const TCHAR* LibDescription()
{
    return _T("UnityMeshSync for 3ds Max (Release " msPluginVersionStr ") (Unity Technologies)");
}

msmaxAPI int LibNumberClasses()
{
    return 0;
}

msmaxAPI ClassDesc* LibClassDesc(int i)
{
    return nullptr;
}

msmaxAPI ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

msmaxAPI ULONG CanAutoDefer()
{
    return 0;
}


#include <maxscript\macros\define_instantiation_functions.h>
def_struct_primitive(Window, UnityMeshSync, "Window");
def_struct_primitive(ServerStatus, UnityMeshSync, "ServerStatus");
def_struct_primitive(Settings, UnityMeshSync, "Settings");
def_struct_primitive(Send, UnityMeshSync, "Send");
def_struct_primitive(Import, UnityMeshSync, "Import");
def_struct_primitive(ExportCache, UnityMeshSync, "ExportCache");

Value* Window_cf(Value** arg_list, int count)
{
    if (count >= 1 && wcscmp(arg_list[0]->to_string(), L"close") == 0) {
        msmaxGetContext().closeSettingWindow();
    }
    else {
        msmaxGetContext().openSettingWindow();
    }
    return &ok;
}

Value* ServerStatus_cf(Value** arg_list, int count)
{
    if (count == 0) {
        return bool_result(msmaxGetContext().isServerAvailable());
    }
    else {
        std::wstring k = arg_list[0]->to_string();
        if (k == L"isAvailable")
            return bool_result(msmaxGetContext().isServerAvailable());
        else if (k == L"errorMessage")
            return new String(ms::ToWCS(msmaxGetContext().getErrorMessage()).c_str());
    }
    return &undefined;
}

Value* Settings_cf(Value** arg_list, int count)
{
    struct GetterSetter
    {
        using getter_t = std::function<Value*()>;
        using setter_t = std::function<void(Value*)>;
        getter_t getter;
        setter_t setter;
    };

    static std::map<std::wstring, GetterSetter> s_table;
    if (s_table.empty()) {
#define Entry(Name, From, To) s_table[L#Name] = {\
    []() -> Value* { return From(msmaxGetSettings().Name); },\
    [](Value *v) -> void{ msmaxGetSettings().Name = v->To(); } }

        Entry(timeout_ms,           Integer::intern, to_int);
        Entry(scale_factor,         Float::intern, to_float);
        Entry(auto_sync,            bool_result, to_bool);

        Entry(sync_meshes,          bool_result, to_bool);
        Entry(sync_normals,         bool_result, to_bool);
        Entry(sync_uvs,             bool_result, to_bool);
        Entry(sync_colors,          bool_result, to_bool);
        Entry(flip_faces,           bool_result, to_bool);
        Entry(make_double_sided,    bool_result, to_bool);
        Entry(ignore_non_renderable,bool_result, to_bool);
        Entry(bake_modifiers,       bool_result, to_bool);
        Entry(use_render_meshes,    bool_result, to_bool);
        Entry(flatten_hierarchy,    bool_result, to_bool);

        Entry(sync_bones,           bool_result, to_bool);
        Entry(sync_blendshapes,     bool_result, to_bool);
        Entry(sync_cameras,         bool_result, to_bool);
        Entry(sync_lights,          bool_result, to_bool);
        Entry(sync_textures,        bool_result, to_bool);

        Entry(frame_step,           Float::intern, to_float);

        Entry(multithreaded,        bool_result, to_bool);

#undef Entry
    }

    auto& settings = msmaxGetSettings();
    if (count == 1) {
        one_value_local(result);
        auto it = s_table.find(arg_list[0]->to_string());
        if (it != s_table.end()) {
            vl.result = it->second.getter();
            return_value(vl.result);
        }
        else {
            return &undefined;
        }
    }
    else {
        for (int i = 0; i + 1 < count; /**/) {
            auto it = s_table.find(arg_list[i++]->to_string());
            if (it != s_table.end()) {
                it->second.setter(arg_list[i++]);
            }
        }
    }
    return &ok;
}

Value* Send_cf(Value** arg_list, int count)
{
    auto target = ExportTarget::Objects;
    auto scope = ObjectScope::All;

    // parse args
    for (int i = 0; i < count; /**/) {
        std::wstring name = arg_list[i++]->to_string();
        if (i + 1 <= count) {
            if (name == L"target") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"objects")
                    target = ExportTarget::Objects;
                else if (value == L"materials")
                    target = ExportTarget::Materials;
                else if (value == L"animations")
                    target = ExportTarget::Animations;
                else if (value == L"everything")
                    target = ExportTarget::Everything;
            }
            else if (name == L"scope") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"all")
                    scope = ObjectScope::All;
                else if (value == L"selected")
                    scope = ObjectScope::Selected;
                else if (value == L"updated")
                    scope = ObjectScope::Updated;
            }
        }
    }

    // do send
    msmaxSendScene(target, scope);
    return &ok;
}

Value* Import_cf(Value** arg_list, int count)
{
    msmaxGetContext().recvScene();
    return &ok;
}

// e.g. UnityMeshSync.ExportCache path:"C:/tmp/hoge.sc" frame_range:1
Value* ExportCache_cf(Value** arg_list, int count)
{
    CacheSettings settings;

    // parse args
    for (int i = 0; i < count; /**/) {
        std::wstring name = arg_list[i++]->to_string();
        if (i + 1 <= count) {
            if      (name == L"path")                   settings.path = mu::ToMBS(arg_list[i++]->to_string());
            else if (name == L"objectScope")            settings.object_scope = (ObjectScope)arg_list[i++]->to_int();
            else if (name == L"frameRange")             settings.frame_range = (FrameRange)arg_list[i++]->to_int();
            else if (name == L"materialFrameRange")     settings.material_frame_range = (MaterialFrameRange)arg_list[i++]->to_int();
            else if (name == L"frameBegin")             settings.frame_begin = arg_list[i++]->to_int();
            else if (name == L"frameEnd")               settings.frame_end = arg_list[i++]->to_int();
            else if (name == L"frameStep")              settings.frame_step = arg_list[i++]->to_float();
            else if (name == L"zstdCompressionLevel")   settings.zstd_compression_level = arg_list[i++]->to_int();
            else if (name == L"ignoreNonRenderable")    settings.ignore_non_renderable = arg_list[i++]->to_bool();
            else if (name == L"makeDoubleSided")        settings.make_double_sided = arg_list[i++]->to_bool();
            else if (name == L"bakeModifiers")          settings.bake_modifiers = arg_list[i++]->to_bool();
            else if (name == L"bakeTransform")          settings.bake_transform = arg_list[i++]->to_bool();
            else if (name == L"useRenderMeshes")        settings.use_render_meshes = arg_list[i++]->to_bool();
            else if (name == L"flattenHierarchy")       settings.flatten_hierarchy = arg_list[i++]->to_bool();
            else if (name == L"mergeMeshes")            settings.merge_meshes = arg_list[i++]->to_bool();
            else if (name == L"stripNormals")           settings.strip_normals = arg_list[i++]->to_bool();
            else if (name == L"stripTangents")          settings.strip_tangents = arg_list[i++]->to_bool();
        }
    }
    msmaxGetContext().exportCache(settings);
    return &ok;
}
