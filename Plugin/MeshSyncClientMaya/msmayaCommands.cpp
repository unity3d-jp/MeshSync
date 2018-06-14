#define _MApiVersion
#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "msmayaCommands.h"


template<class T> void get_arg(T& dst, const char *name, MArgParser& args);

template<> void get_arg(std::string& dst, const char *name, MArgParser& args)
{
    MString tmp;
    args.getFlagArgument(name, 0, tmp);
    dst = tmp.asChar();
}
template<> void get_arg(bool& dst, const char *name, MArgParser& args)
{
    args.getFlagArgument(name, 0, dst);
}
template<> void get_arg(int& dst, const char *name, MArgParser& args)
{
    args.getFlagArgument(name, 0, dst);
}
template<> void get_arg(uint16_t& dst, const char *name, MArgParser& args)
{
    int tmp;
    args.getFlagArgument(name, 0, tmp);
    dst = (uint16_t)tmp;
}
template<> void get_arg(float& dst, const char *name, MArgParser& args)
{
    double tmp;
    args.getFlagArgument(name, 0, tmp);
    dst = (float)tmp;
}


template<class T> void to_MString(MString& dst, const T& value);

template<> void to_MString(MString& dst, const std::string& value)
{
    dst += value.c_str();
}
template<> void to_MString(MString& dst, const bool& value)
{
    dst += (int)value;
}
template<> void to_MString(MString& dst, const int& value)
{
    dst += value;
}
template<> void to_MString(MString& dst, const uint16_t& value)
{
    dst += (int)value;
}
template<> void to_MString(MString& dst, const float& value)
{
    dst += value;
}


void* CmdSettings::create()
{
    return new CmdSettings();
}

const char* CmdSettings::name()
{
    return "UnityMeshSync_Settings";
}


MSyntax CmdSettings::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(true);
    syntax.enableEdit(false);
    syntax.addFlag("-a", "-address", MSyntax::kString);
    syntax.addFlag("-p", "-port", MSyntax::kLong);
    syntax.addFlag("-sf", "-scaleFactor", MSyntax::kDouble);
    syntax.addFlag("-as", "-autosync", MSyntax::kBoolean);
    syntax.addFlag("-sm", "-syncMeshes", MSyntax::kBoolean);
    syntax.addFlag("-smn", "-syncNormals", MSyntax::kBoolean);
    syntax.addFlag("-smu", "-syncUVs", MSyntax::kBoolean);
    syntax.addFlag("-smc", "-syncColors", MSyntax::kBoolean);
    syntax.addFlag("-sms", "-syncBlendShapes", MSyntax::kBoolean);
    syntax.addFlag("-smb", "-syncBones", MSyntax::kBoolean);
    syntax.addFlag("-sc", "-syncCameras", MSyntax::kBoolean);
    syntax.addFlag("-sl", "-syncLights", MSyntax::kBoolean);
    syntax.addFlag("-sco", "-syncConstraints", MSyntax::kBoolean);
    syntax.addFlag("-ats", "-animationTS", MSyntax::kDouble);
    syntax.addFlag("-asp", "-animationSPS", MSyntax::kLong);
    syntax.addFlag("-tw", "-applyTweak", MSyntax::kBoolean);
    syntax.addFlag("-mt", "-multithreaded", MSyntax::kBoolean);

    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& settings = MeshSyncClientMaya::getInstance().m_settings;

    MString result;
#define Handle(Name, Value)\
    if (args.isFlagSet(Name)) {\
        if(args.isQuery()) to_MString(result, Value);\
        else get_arg(Value, Name, args);\
    }

    Handle("address", settings.client_settings.server);
    Handle("port", settings.client_settings.port);
    Handle("scaleFactor", settings.scale_factor);
    Handle("autosync", settings.auto_sync);
    Handle("syncMeshes", settings.sync_meshes);
    Handle("syncNormals", settings.sync_normals);
    Handle("syncUVs", settings.sync_uvs);
    Handle("syncColors", settings.sync_colors);
    Handle("syncBlendShapes", settings.sync_blendshapes);
    Handle("syncBones", settings.sync_bones);
    Handle("syncCameras", settings.sync_cameras);
    Handle("syncLights", settings.sync_lights);
    Handle("syncConstraints", settings.sync_constraints);
    Handle("animationTS", settings.animation_time_scale);
    Handle("animationSPS", settings.animation_sps);
    Handle("applyTweak", settings.apply_tweak);
    Handle("multithreaded", settings.multithreaded);
#undef Handle

    MPxCommand::setResult(result);
    return MStatus::kSuccess;
}


void* CmdExport::create()
{
    return new CmdExport();
}
const char* CmdExport::name()
{
    return "UnityMeshSync_Export";
}

MSyntax CmdExport::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(false);
    syntax.enableEdit(false);
    syntax.addFlag("-s", "-scope", MSyntax::kString);
    syntax.addFlag("-t", "-target", MSyntax::kString);
    return syntax;
}

MStatus CmdExport::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& instance = MeshSyncClientMaya::getInstance();

    bool animations = false;
    auto scope = MeshSyncClientMaya::SendScope::All;

    if (args.isFlagSet("target")) {
        std::string t;
        get_arg(t, "target", args);
        if (t == "animations")
            animations = true;
    }

    if (args.isFlagSet("scope")) {
        std::string s;
        get_arg(s, "scope", args);
        if (s == "selection")
            scope = MeshSyncClientMaya::SendScope::Selected;
        else if (s == "updated")
            scope = MeshSyncClientMaya::SendScope::Updated;
    }

    if (animations)
        MeshSyncClientMaya::getInstance().sendAnimations(scope);
    else
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

MSyntax CmdImport::createSyntax()
{
    MSyntax syntax;
    return syntax;
}

MStatus CmdImport::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().recvScene();
    return MStatus::kSuccess;
}
