#define _MApiVersion
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
MStatus CmdSettings::doIt(const MArgList& args)
{
    for (uint32_t i = 0; i < args.length(); ++i) {
        if (args.asString(i) == MString("-address"))
            MeshSyncClientMaya::getInstance().setServerAddress(args.asString(++i).asChar());
        else if (args.asString(i) == MString("-port"))
            MeshSyncClientMaya::getInstance().setServerPort((uint16_t)args.asInt(++i));
        else if (args.asString(i) == MString("-autosync"))
            MeshSyncClientMaya::getInstance().setAutoSync(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncMeshes"))
            MeshSyncClientMaya::getInstance().setSyncMeshes(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncBlendshapes"))
            MeshSyncClientMaya::getInstance().setSyncBlendShapes(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncBones"))
            MeshSyncClientMaya::getInstance().setSyncBones(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncCameras"))
            MeshSyncClientMaya::getInstance().setSyncCameras(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncLights"))
            MeshSyncClientMaya::getInstance().setSyncLights(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-syncAnimations"))
            MeshSyncClientMaya::getInstance().setSyncAnimations(args.asInt(++i) != 0);
        else if (args.asString(i) == MString("-animationSPS"))
            MeshSyncClientMaya::getInstance().setAnimationSPS(args.asInt(++i) != 0);
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
MStatus CmdSync::doIt(const MArgList& args)
{
    auto scope = MeshSyncClientMaya::TargetScope::All;
    for (uint32_t i = 0; i < args.length(); ++i) {
        if (args.asString(i) == MString("-scope")) {
            auto s = args.asString(i++);
            if (s == MString("selection")) {
                scope = MeshSyncClientMaya::TargetScope::Selection;
            }
        }
    }
    MeshSyncClientMaya::getInstance().sendScene(scope);
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
MStatus CmdImport::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().importScene();
    return MStatus::kSuccess;
}
