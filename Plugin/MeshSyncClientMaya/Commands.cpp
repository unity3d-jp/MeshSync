#define _MApiVersion
#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"


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


template<class T> void to_ms(MString& dst, T& value);

template<> void to_ms(MString& dst, std::string& value)
{
    dst += value.c_str();
}
template<> void to_ms(MString& dst, bool& value)
{
    dst += (int)value;
}
template<> void to_ms(MString& dst, int& value)
{
    dst += value;
}
template<> void to_ms(MString& dst, uint16_t& value)
{
    dst += (int)value;
}
template<> void to_ms(MString& dst, float& value)
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
    syntax.addFlag("-a",    "-address",         MSyntax::kString);
    syntax.addFlag("-p",    "-port",            MSyntax::kLong);
    syntax.addFlag("-sf",   "-scaleFactor",     MSyntax::kDouble);
    syntax.addFlag("-as",   "-autosync",        MSyntax::kBoolean);
    syntax.addFlag("-sm",   "-syncMeshes",      MSyntax::kBoolean);
    syntax.addFlag("-smn",  "-syncNormals",     MSyntax::kBoolean);
    syntax.addFlag("-smu",  "-syncUVs",         MSyntax::kBoolean);
    syntax.addFlag("-smc",  "-syncColors",      MSyntax::kBoolean);
    syntax.addFlag("-sms",  "-syncBlendShapes", MSyntax::kBoolean);
    syntax.addFlag("-smb",  "-syncBones",       MSyntax::kBoolean);
    syntax.addFlag("-sc",   "-syncCameras",     MSyntax::kBoolean);
    syntax.addFlag("-sl",   "-syncLights",      MSyntax::kBoolean);
    syntax.addFlag("-sa",   "-syncAnimations",  MSyntax::kBoolean);
    syntax.addFlag("-sps",  "-animationSPS",    MSyntax::kLong);
    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& instance = MeshSyncClientMaya::getInstance();

    MString result;
#define Handle(Name, Value)\
    if (args.isFlagSet(Name)) {\
        if(args.isQuery()) to_ms(result, Value);\
        else get_arg(Value, Name, args);\
    }

    Handle("address", instance.m_client_settings.server);
    Handle("port", instance.m_client_settings.port);
    Handle("scaleFactor", instance.m_scale_factor);
    Handle("autosync", instance.m_auto_sync);
    Handle("syncMeshes", instance.m_sync_meshes);
    Handle("syncNormals", instance.m_sync_normals);
    Handle("syncUVs", instance.m_sync_uvs);
    Handle("syncColors", instance.m_sync_colors);
    Handle("syncBlendShapes", instance.m_sync_blendshapes);
    Handle("syncBones", instance.m_sync_bones);
    Handle("syncCameras", instance.m_sync_cameras);
    Handle("syncLights", instance.m_sync_lights);
    Handle("syncAnimations", instance.m_sync_animations);
    Handle("animationSPS", instance.m_animation_sps);
#undef Handle

    MPxCommand::setResult(result);
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

MSyntax CmdSync::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(false);
    syntax.enableEdit(false);
    syntax.addFlag("-s", "-scope", MSyntax::kString);
    return syntax;
}

MStatus CmdSync::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& instance = MeshSyncClientMaya::getInstance();

    auto scope = MeshSyncClientMaya::TargetScope::All;
    if (args.isFlagSet("scope")) {
        std::string s;
        get_arg(s, "scope", args);
        if (s == "selection")
            scope = MeshSyncClientMaya::TargetScope::Selection;
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

MSyntax CmdImport::createSyntax()
{
    MSyntax syntax;
    return syntax;
}

MStatus CmdImport::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().importScene();
    return MStatus::kSuccess;
}
