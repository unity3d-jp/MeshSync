#define _MApiVersion
#include "pch.h"
#include "msmayaContext.h"
#include "msmayaCommand.h"


template<class T> static void get_arg(T& dst, const char *name, MArgParser& args);

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


template<class T> static void to_MString(MString& dst, const T& value);

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

template<class T> static void set_result(const T& value);

template<> void set_result(const std::string& value)
{
    MPxCommand::setResult(value.c_str());
}
template<> void set_result(const bool& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const int& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const uint16_t& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const float& value)
{
    MPxCommand::setResult(value);
}


void* CmdServerStatus::create()
{
    return new CmdServerStatus();
}

const char* CmdServerStatus::name()
{
    return "UnityMeshSync_ServerStatus";
}

MSyntax CmdServerStatus::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(true);
    syntax.enableEdit(false);
    syntax.addFlag("-ia", "-isAvailable", MSyntax::kBoolean);
    syntax.addFlag("-em", "-errorMessage", MSyntax::kString);
    return syntax;
}

MStatus CmdServerStatus::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    if (!args.isQuery())
        return MStatus::kFailure;

    auto& ctx = msmayaGetContext();
    if (args.isFlagSet("isAvailable"))
        set_result(ctx.isServerAvailable());
    else if (args.isFlagSet("errorMessage"))
        set_result(std::string("MeshSync: ") + ctx.getErrorMessage() + "\n");
    return MStatus::kSuccess;
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
    syntax.addFlag("-plv", "-pluginVersion", MSyntax::kString);
    syntax.addFlag("-prv", "-protocolVersion", MSyntax::kString);
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
    syntax.addFlag("-kfc", "-keepFlatCurves", MSyntax::kBoolean);
    syntax.addFlag("-rn", "-removeNamespace", MSyntax::kBoolean);
    syntax.addFlag("-mt", "-multithreaded", MSyntax::kBoolean);
    syntax.addFlag("-fct", "-fbxCompatibleTransform", MSyntax::kBoolean);
    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& settings = msmayaGetSettings();

    if (args.isFlagSet("pluginVersion")) {
        if (args.isQuery())
            set_result(std::string(msPluginVersionStr));
    }
    else if (args.isFlagSet("protocolVersion")) {
        if (args.isQuery())
            set_result(std::to_string(msProtocolVersion));
    }

#define Handle(Name, Value, Sync)\
    if (args.isFlagSet(Name)) {\
        if(args.isQuery()) {\
            set_result(Value);\
        }\
        else {\
            get_arg(Value, Name, args);\
            if (settings.auto_sync && Sync)\
                msmayaGetContext().sendObjects(msmayaContext::SendScope::All, false);\
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
    Handle("keepFlatCurves", settings.keep_flat_curves, false);
    Handle("removeNamespace", settings.remove_namespace, true);
    Handle("multithreaded", settings.multithreaded, false);
    Handle("fbxCompatibleTransform", settings.fbx_compatible_transform, true);
#undef Handle

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

    auto target = msmayaContext::SendTarget::Objects;
    auto scope = msmayaContext::SendScope::All;

    // parse args
    if (args.isFlagSet("target")) {
        std::string t;
        get_arg(t, "target", args);
        if (t == "objects")
            target = msmayaContext::SendTarget::Objects;
        else if (t == "materials")
            target = msmayaContext::SendTarget::Materials;
        else if (t == "animations")
            target = msmayaContext::SendTarget::Animations;
        else if (t == "everything")
            target = msmayaContext::SendTarget::Everything;
    }
    if (args.isFlagSet("scope")) {
        std::string s;
        get_arg(s, "scope", args);
        if (s == "all")
            scope = msmayaContext::SendScope::All;
        else if (s == "selection")
            scope = msmayaContext::SendScope::Selected;
        else if (s == "updated")
            scope = msmayaContext::SendScope::Updated;
    }

    // do send
    auto& inst = msmayaGetContext();
    if (!inst.isServerAvailable()) {
        inst.logError("MeshSync: Server not available. %s", inst.getErrorMessage().c_str());
        return MStatus::kFailure;
    }

    if (target == msmayaContext::SendTarget::Objects) {
        inst.wait();
        inst.sendObjects(scope, true);
    }
    else if (target == msmayaContext::SendTarget::Materials) {
        inst.wait();
        inst.sendMaterials(true);
    }
    else if (target == msmayaContext::SendTarget::Animations) {
        inst.wait();
        inst.sendAnimations(scope);
    }
    else if (target == msmayaContext::SendTarget::Everything) {
        inst.wait();
        inst.sendMaterials(true);
        inst.wait();
        inst.sendObjects(scope, true);
        inst.wait();
        inst.sendAnimations(scope);
    }
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
    msmayaGetContext().recvObjects();
    return MStatus::kSuccess;
}
