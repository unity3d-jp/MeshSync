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
    bool tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = tmp;
}
template<> void get_arg(int& dst, const char *name, MArgParser& args)
{
    int tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = tmp;
}
template<> void get_arg(uint16_t& dst, const char *name, MArgParser& args)
{
    int tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = (uint16_t)tmp;
}
template<> void get_arg(float& dst, const char *name, MArgParser& args)
{
    double tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
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
    syntax.addFlag("-v", "-version", MSyntax::kString);
    syntax.addFlag("-a", "-address", MSyntax::kString);
    syntax.addFlag("-p", "-port", MSyntax::kLong);
    syntax.addFlag("-sf", "-scaleFactor", MSyntax::kDouble);
    syntax.addFlag("-as", "-autosync", MSyntax::kBoolean);
    syntax.addFlag("-sm", "-syncMeshes", MSyntax::kBoolean);
    syntax.addFlag("-smn", "-syncNormals", MSyntax::kBoolean);
    syntax.addFlag("-smu", "-syncUVs", MSyntax::kBoolean);
    syntax.addFlag("-smc", "-syncColors", MSyntax::kBoolean);
    syntax.addFlag("-mbs", "-makeDoubleSided", MSyntax::kBoolean);
    syntax.addFlag("-bd", "-bakeDeformers", MSyntax::kBoolean);
    syntax.addFlag("-tw", "-applyTweak", MSyntax::kBoolean);
    syntax.addFlag("-sms", "-syncBlendShapes", MSyntax::kBoolean);
    syntax.addFlag("-smb", "-syncBones", MSyntax::kBoolean);
    syntax.addFlag("-stx", "-syncTextures", MSyntax::kBoolean);
    syntax.addFlag("-sc", "-syncCameras", MSyntax::kBoolean);
    syntax.addFlag("-sl", "-syncLights", MSyntax::kBoolean);
    syntax.addFlag("-sco", "-syncConstraints", MSyntax::kBoolean);
    syntax.addFlag("-ats", "-animationTS", MSyntax::kDouble);
    syntax.addFlag("-asp", "-animationSPS", MSyntax::kDouble);
    syntax.addFlag("-kfr", "-keyframeReduction", MSyntax::kBoolean);
    syntax.addFlag("-rn", "-removeNamespace", MSyntax::kBoolean);
    syntax.addFlag("-mt", "-multithreaded", MSyntax::kBoolean);
    syntax.addFlag("-fct", "-fbxCompatibleTransform", MSyntax::kBoolean);

    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& settings = MeshSyncClientMaya::getInstance().m_settings;

    MString result;

    if (args.isFlagSet("version")) {
        if (args.isQuery()) to_MString(result, std::string(msReleaseDateStr));
    }

#define Handle(Name, Value, SendIfAutosync)\
    if (args.isFlagSet(Name)) {\
        if(args.isQuery()) to_MString(result, Value);\
        else {\
            get_arg(Value, Name, args);\
            if(settings.auto_sync && SendIfAutosync) MeshSyncClientMaya::getInstance().sendScene(MeshSyncClientMaya::SendScope::All, false);\
        }\
    }

    Handle("address", settings.client_settings.server, false);
    Handle("port", settings.client_settings.port, false);
    Handle("scaleFactor", settings.scale_factor, true);
    Handle("autosync", settings.auto_sync, true);
    Handle("syncMeshes", settings.sync_meshes, true);
    Handle("syncNormals", settings.sync_normals, true);
    Handle("syncUVs", settings.sync_uvs, true);
    Handle("syncColors", settings.sync_colors, true);
    Handle("makeDoubleSided", settings.make_double_sided, true);
    Handle("bakeDeformers", settings.bake_deformers, true);
    Handle("applyTweak", settings.apply_tweak, true);
    Handle("syncBlendShapes", settings.sync_blendshapes, true);
    Handle("syncBones", settings.sync_bones, true);
    Handle("syncTextures", settings.sync_textures, true);
    Handle("syncCameras", settings.sync_cameras, true);
    Handle("syncLights", settings.sync_lights, true);
    Handle("syncConstraints", settings.sync_constraints, true);
    Handle("animationTS", settings.animation_time_scale, false);
    Handle("animationSPS", settings.animation_sps, false);
    Handle("keyframeReduction", settings.reduce_keyframes, false);
    Handle("removeNamespace", settings.remove_namespace, true);
    Handle("multithreaded", settings.multithreaded, false);
    Handle("fbxCompatibleTransform", settings.fbx_compatible_transform, true);
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
    syntax.addFlag("-fa", "-dirtyAll", MSyntax::kBoolean);
    return syntax;
}

MStatus CmdExport::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& instance = MeshSyncClientMaya::getInstance();

    bool dirty_all = true;
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
        if (s == "selection") {
            scope = MeshSyncClientMaya::SendScope::Selected;
            dirty_all = false;
        }
        else if (s == "updated") {
            scope = MeshSyncClientMaya::SendScope::Updated;
            dirty_all = false;
        }
    }
    if (args.isFlagSet("dirtyAll")) {
        get_arg(dirty_all, "dirtyAll", args);
    }

    if (animations)
        MeshSyncClientMaya::getInstance().sendAnimations(scope);
    else
        MeshSyncClientMaya::getInstance().sendScene(scope, dirty_all);
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
