#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"


void* CmdSettings::create()
{
    return new CmdSettings();
}

const char* CmdSettings::name()
{
    return "UnityMeshSync_Settings";
}

MSyntax CmdSettings::syntax()
{
    MSyntax syntax;
    syntax.addFlag("-a", "-address", MSyntax::kString);
    syntax.addFlag("-p", "-port", MSyntax::kLong);
    syntax.addFlag("-s", "-autosync", MSyntax::kLong);
    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args)
{
    MArgParser parser(syntax(), args);
    {
        MString addr;
        if (parser.getCommandArgument(0, addr) == MStatus::kSuccess) {
            MeshSyncClientMaya::getInstance().setServerAddress(addr.asChar());
        }
    }
    {
        int port;
        if (parser.getCommandArgument(1, port) == MStatus::kSuccess) {
            MeshSyncClientMaya::getInstance().setServerPort((uint16_t)port);
        }
    }
    {
        int v;
        if (parser.getCommandArgument(2, v) == MStatus::kSuccess) {
            MeshSyncClientMaya::getInstance().setAutoSync(v != 0);
        }
    }
    return MStatus::kSuccess;
}


void* CmdSync::create()
{
    return new CmdSync();
}
const char* CmdSync::name()
{
    return "UnityMeshSync_Sync";
}
MSyntax CmdSync::syntax()
{
    MSyntax syntax;
    syntax.addFlag("-m", "-mode", MSyntax::kString);
    return syntax;
}
MStatus CmdSync::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().sendScene();
    return MStatus::kSuccess;
}


void* CmdImport::create()
{
    return new CmdImport();
}
const char* CmdImport::name()
{
    return "UnityMeshSync_Import";
}
MSyntax CmdImport::syntax()
{
    return MSyntax();
}
MStatus CmdImport::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().importScene();
    return MStatus::kSuccess;
}
