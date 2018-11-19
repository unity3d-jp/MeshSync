#include "pch.h"
#include "MeshSyncClient3dsMax.h"

HINSTANCE g_msmax_hinstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_msmax_hinstance = hinstDLL;
        MaxSDK::Util::UseLanguagePackLocale();
        msmaxInstance(); // initialize instance
        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    return(TRUE);
}

msmaxAPI const TCHAR* LibDescription()
{
    return _T("UnityMeshSync for 3ds Max (Release " msReleaseDateStr ") (Unity Technologies)");
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

def_visible_primitive(UnityMeshSync_Settings, "UnityMeshSync_Settings");
Value* UnityMeshSync_Settings_cf(Value** arg_list, int count)
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
    []() -> Value* { return From(msmaxInstance().getSettings().Name); },\
    [](Value *v) -> void{ msmaxInstance().getSettings().Name = v->To(); } }
        Entry(timeout_ms,           Integer::intern, to_int);
        Entry(scale_factor,         Float::intern, to_float);
        Entry(animation_time_scale, Float::intern, to_float);
        Entry(animation_sps,        Float::intern, to_float);
        Entry(auto_sync,            bool_result, to_bool);
        Entry(sync_meshes,          bool_result, to_bool);
        Entry(sync_normals,         bool_result, to_bool);
        Entry(sync_uvs,             bool_result, to_bool);
        Entry(sync_colors,          bool_result, to_bool);
        Entry(sync_bones,           bool_result, to_bool);
        Entry(sync_blendshapes,     bool_result, to_bool);
        Entry(sync_cameras,         bool_result, to_bool);
        Entry(sync_lights,          bool_result, to_bool);
        Entry(multithreaded,        bool_result, to_bool);
#undef Entry
    }

    auto& settings = msmaxInstance().getSettings();
    if (count >= 2 && wcscmp(arg_list[0]->to_string(), L"-q") == 0) {
        one_value_local(result);
        auto it = s_table.find(arg_list[1]->to_string());
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

def_visible_primitive(UnityMeshSync_ExportScene, "UnityMeshSync_Export");
Value* UnityMeshSync_ExportScene_cf(Value** arg_list, int count)
{
    bool animations = false;
    auto scope = MeshSyncClient3dsMax::SendScope::All;
    bool force_all = true;

    for (int i = 0; i < count; /**/) {
        std::wstring name = arg_list[i++]->to_string();
        if (i + 1 < count) {
            if (name == L"-target") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"animations")
                    animations = true;
            }
            else if (name == L"-scope") {
                std::wstring value = arg_list[i++]->to_string();
                if (value == L"selected")
                    scope = MeshSyncClient3dsMax::SendScope::Selected;
                else if (value == L"updated")
                    scope = MeshSyncClient3dsMax::SendScope::Updated;
            }
            else if (name == L"-force_all") {
                std::wstring value = arg_list[i++]->to_string();
                force_all = value == L"true" || value == L"1";
            }
        }
    }

    if (animations)
        msmaxInstance().sendAnimations(scope);
    else
        msmaxInstance().sendScene(scope, force_all);
    return &ok;
}

def_visible_primitive(UnityMeshSync_Import, "UnityMeshSync_Import");
Value* UnityMeshSync_Import_cf(Value** arg_list, int count)
{
    msmaxInstance().recvScene();
    return &ok;
}

def_visible_primitive(UnityMeshSync, "UnityMeshSync");
Value* UnityMeshSync_cf(Value** arg_list, int count)
{
    if (count >= 1 && wcscmp(arg_list[0]->to_string(), L"close") == 0) {
        msmaxInstance().closeWindow();
    }
    else {
        msmaxInstance().openWindow();
    }
    return &ok;
}

