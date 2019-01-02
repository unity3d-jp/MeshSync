#include "pch.h"
#include "msmqUtils.h"

std::string BuildPath(MQDocument doc, MQObject obj)
{
    std::string ret;
    if (auto parent = doc->GetParentObject(obj)) {
        ret += BuildPath(doc, parent);
    }
    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    ret += "/";
    ret += name;
    return ret;
}

bool ExtractID(const char *name, int& id)
{
    if (auto p = std::strstr(name, "[id:")) {
        if (sscanf(p, "[id:%08x]", &id) == 1) {
            return true;
        }
    }
    return false;
}

float3 ToEular(const MQAngle& ang, bool flip_head)
{
    if (flip_head) {
        return float3{
            ang.pitch,
            -ang.head + 180.0f, // I can't explain why this modification is needed...
            ang.bank
        } *mu::Deg2Rad;
    }
    else {
        return float3{
            ang.pitch,
            ang.head,
            ang.bank
        } *mu::Deg2Rad;
    }
}

quatf ToQuaternion(const MQAngle& ang)
{
    return rotateZXY(ToEular(ang));
}

void ExtractLocalTransform(MQObject obj, float3& pos, quatf& rot, float3& scale)
{
    pos = to_float3(obj->GetTranslation());
    rot = ToQuaternion(obj->GetRotation());
    scale = to_float3(obj->GetScaling());
}

float4x4 ExtractGlobalMatrix(MQDocument doc, MQObject obj)
{
    auto mat = to_float4x4(obj->GetLocalMatrix());
    if (auto parent = doc->GetParentObject(obj)) {
        auto pmat = ExtractGlobalMatrix(doc, parent);
        mat = mat * pmat;
    }
    return mat;
}
