#include "pch.h"
#include "MeshSyncClient3dsMax.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        MaxSDK::Util::UseLanguagePackLocale();
        MeshSyncClient3dsMax::getInstance(); // initialize instance
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


def_visible_primitive(UnityMeshSync_ExportScene, "UnityMeshSync_ExportScene");
Value* UnityMeshSync_ExportScene_cf(Value** arg_list, int count)
{
    MeshSyncClient3dsMax::getInstance().sendScene(MeshSyncClient3dsMax::SendScope::All);
    return &ok;
}

def_visible_primitive(UnityMeshSync_ExportAnimations, "UnityMeshSync_ExportAnimations");
Value* UnityMeshSync_ExportAnimations_cf(Value** arg_list, int count)
{
    MeshSyncClient3dsMax::getInstance().sendAnimations(MeshSyncClient3dsMax::SendScope::All);
    return &ok;
}

