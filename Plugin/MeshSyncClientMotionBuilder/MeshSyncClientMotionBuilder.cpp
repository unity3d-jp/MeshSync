#include "pch.h"
#include "MeshSyncClientMotionBuilder.h"

#ifdef _WIN32
#pragma comment(lib, "fbsdk.lib")
#endif


FBLibraryDeclare(msmbDevice)
{
    FBLibraryRegister(msmbDevice);
    FBLibraryRegister(msmbLayout);
}
FBLibraryDeclareEnd;

bool FBLibrary::LibInit() { return true; }
bool FBLibrary::LibOpen() { return true; }
bool FBLibrary::LibReady() { return true; }
bool FBLibrary::LibClose() { return true; }
bool FBLibrary::LibRelease() { return true; }


