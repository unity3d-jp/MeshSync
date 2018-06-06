#define _MApiVersion
#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "msmayaCommands.h"


template<class T> void get_arg(T& dst, const MArgList& args, uint32_t i);

template<> void get_arg(std::string& dst, const MArgList& args, uint32_t i)
{
    MString tmp;
    args.get(i, tmp);
    dst = tmp.asChar();
}
template<> void get_arg(bool& dst, const MArgList& args, uint32_t i)
{
    args.get(i, dst);
}
template<> void get_arg(int& dst, const MArgList& args, uint32_t i)
{
    args.get(i, dst);
}
template<> void get_arg(uint16_t& dst, const MArgList& args, uint32_t i)
{
    int tmp;
    args.get(i, tmp);
    dst = (uint16_t)tmp;
}
template<> void get_arg(float& dst, const MArgList& args, uint32_t i)
{
    double tmp;
    args.get(i, tmp);
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

MStatus CmdSettings::doIt(const MArgList& args)
{
    MString result;
    auto& settings = MeshSyncClientMaya::getInstance().m_settings;

    // parse args
    uint32_t len = args.length();
    if (len > 0) {
        uint32_t i = 0;
        bool query = false;

        MString name;
        {
            args.get(0, name);
            if (name == "-q") {
                ++i;
                query = true;
            }
        }
        for (; i < len; ) {
            args.get(i++, name);
#define Body(Name, Value)\
            if (name == Name) {\
                if(query) { to_MString(result, Value); break; }\
                else if (i < len) { get_arg(Value, args, i++); }\
            }

            Body("-address", settings.client_settings.server);
            Body("-port", settings.client_settings.port);
            Body("-scaleFactor", settings.scale_factor);
            Body("-autosync", settings.auto_sync);
            Body("-syncMeshes", settings.sync_meshes);
            Body("-syncNormals", settings.sync_normals);
            Body("-syncUVs", settings.sync_uvs);
            Body("-syncColors", settings.sync_colors);
            Body("-syncBlendShapes", settings.sync_blendshapes);
            Body("-syncBones", settings.sync_bones);
            Body("-syncCameras", settings.sync_cameras);
            Body("-syncLights", settings.sync_lights);
            Body("-syncConstraints", settings.sync_constraints);
            Body("-animationTS", settings.animation_time_scale);
            Body("-animationSPS", settings.animation_sps);
            Body("-applyTweak", settings.apply_tweak);
            Body("-multithreaded", settings.multithreaded);
#undef Body
        }
    }

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

MStatus CmdExport::doIt(const MArgList& args)
{
    MStatus status;
    auto& instance = MeshSyncClientMaya::getInstance();

    bool animations = false;
    auto scope = MeshSyncClientMaya::SendScope::All;

    // parse args
    uint32_t len = args.length();
    if (len > 0) {
        MString name;
        for (uint32_t i = 0; i < len - 1; ) {
            args.get(i++, name);
            if (name == "-target") {
                std::string t;
                get_arg(t, args, i++);
                if (t == "animations")
                    animations = true;
            }
            else if (name == "-scope") {
                std::string s;
                get_arg(s, args, i++);
                if (s == "selection")
                    scope = MeshSyncClientMaya::SendScope::Selected;
                else if (s == "updated")
                    scope = MeshSyncClientMaya::SendScope::Updated;
            }
        }
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

MStatus CmdImport::doIt(const MArgList&)
{
    MeshSyncClientMaya::getInstance().recvScene();
    return MStatus::kSuccess;
}
