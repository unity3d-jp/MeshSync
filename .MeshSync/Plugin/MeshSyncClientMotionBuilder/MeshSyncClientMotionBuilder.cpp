#include "pch.h"
#include "msmobuDevice.h"
#include "msmobuLayout.h"

#ifdef _WIN32
#pragma comment(lib, "fbsdk.lib")
#endif


FBLibraryDeclare(msmobuDevice)
{
    FBLibraryRegister(msmobuDevice);
    FBLibraryRegister(msmobuLayout);
}
FBLibraryDeclareEnd;

bool FBLibrary::LibInit() { return true; }
bool FBLibrary::LibOpen() { return true; }
bool FBLibrary::LibReady() { return true; }
bool FBLibrary::LibClose() { return true; }
bool FBLibrary::LibRelease() { return true; }


