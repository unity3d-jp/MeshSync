#include "pch.h"
#include "MeshSyncClient3dsMax.h"

#ifdef _WIN32
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "core.lib")
#pragma comment(lib, "geom.lib")
#pragma comment(lib, "gfx.lib")
#pragma comment(lib, "maxutil.lib")
#pragma comment(lib, "mesh.lib")
#endif

static std::unique_ptr<MeshSyncClient3dsMax> g_plugin;

MeshSyncClient3dsMax & MeshSyncClient3dsMax::getInstance()
{
    return *g_plugin;
}

MeshSyncClient3dsMax::MeshSyncClient3dsMax()
{
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
}

void MeshSyncClient3dsMax::update()
{
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::sendAnimations(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::recvScene()
{
    return false;
}


static HINSTANCE g_hinstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_hinstance = hinstDLL;
        MaxSDK::Util::UseLanguagePackLocale();
        g_plugin.reset(new MeshSyncClient3dsMax());
        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    return(TRUE);
}

msmaxAPI const TCHAR* LibDescription()
{
    return _T("UnityMeshSync for 3ds Max");
}

msmaxAPI int LibNumberClasses()
{
    return 1;
}

msmaxAPI ClassDesc* LibClassDesc(int i)
{
    switch (i) {
    //case 0:  return &_3DSDesc;
    default: break;
    }
    return nullptr;
}

msmaxAPI ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

msmaxAPI ULONG CanAutoDefer()
{
    return 1;
}