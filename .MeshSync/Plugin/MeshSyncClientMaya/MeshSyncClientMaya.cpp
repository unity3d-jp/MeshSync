#include "pch.h"
#include "msmayaUtils.h"
#include "msmayaContext.h"
#include "msmayaCommand.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
#endif


void msmayaInitialize(MObject& obj);
void msmayaUninitialize();

MStatus initializePlugin(MObject obj)
{
    msmayaInitialize(obj);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
    msmayaUninitialize();
    return MS::kSuccess;
}
