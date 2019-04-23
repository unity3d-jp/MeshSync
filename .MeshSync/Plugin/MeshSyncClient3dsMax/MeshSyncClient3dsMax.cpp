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
def_struct_primitive(Export, UnityMeshSync, "Export");
def_struct_primitive(Import, UnityMeshSync, "Import");

Value* Window_cf(Value** arg_list, int count)
{
    if (count >= 1 && wcscmp(arg_list[0]->to_string(), L"close") == 0) {
        msmaxGetContext().closeWindow();
    }
    else {
        msmaxGetContext().openWindow();
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
        Entry(bake_modifiers,       bool_result, to_bool);
        Entry(convert_to_mesh,      bool_result, to_bool);

        Entry(sync_bones,           bool_result, to_bool);
        Entry(sync_blendshapes,     bool_result, to_bool);
        Entry(sync_cameras,         bool_result, to_bool);
        Entry(sync_lights,          bool_result, to_bool);
        Entry(sync_textures,        bool_result, to_bool);

        Entry(animation_time_scale, Float::intern, to_float);
        Entry(animation_sps,        Float::intern, to_float);
        Entry(keyframe_reduction,   bool_result, to_bool);
        Entry(keep_flat_curves,     bool_result, to_bool);
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

Value* Export_cf(Value** arg_list, int count)
{
    auto target = msmaxContext::SendTarget::Objects;
    auto scope = msmaxContext::SendScope::All;

    // parse args
    for (int i = 0; i < count; /**/) {
        std::wstring name = arg_list[i++]->to_string();
        if (i + 1 <= count) {
            if (name == L"target") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"objects")
                    target = msmaxContext::SendTarget::Objects;
                else if (value == L"materials")
                    target = msmaxContext::SendTarget::Materials;
                else if (value == L"animations")
                    target = msmaxContext::SendTarget::Animations;
                else if (value == L"everything")
                    target = msmaxContext::SendTarget::Everything;
            }
            else if (name == L"scope") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"all")
                    scope = msmaxContext::SendScope::All;
                else if (value == L"selected")
                    scope = msmaxContext::SendScope::Selected;
                else if (value == L"updated")
                    scope = msmaxContext::SendScope::Updated;
            }
        }
    }

    // do send
    msmaxExport(target, scope);
    return &ok;
}

Value* Import_cf(Value** arg_list, int count)
{
    msmaxGetContext().recvScene();
    return &ok;
}
