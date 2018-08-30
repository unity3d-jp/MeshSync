#include "pch.h"
#include "MeshSyncClientMotionBuilder.h"


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


