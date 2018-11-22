#pragma once

#define MaxNameBuffer 128

using namespace mu;

std::wstring L(const std::string& s)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
}

std::string S(const std::wstring& w)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(w);
}

static inline float4 to_float4(const MQColor& v)
{
    return { v.r, v.g, v.b, 1.0f };
}
static inline float3 to_float3(const MQPoint& v)
{
    return (const float3&)v;
}
static inline float4x4 to_float4x4(const MQMatrix& v)
{
    return (const float4x4&)v;
}
static inline MQColor to_MQColor(const float4& v)
{
    return MQColor(v[0], v[1], v[2]);
}

static inline std::string BuildPath(MQDocument doc, MQObject obj)
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

static inline bool ExtractID(const char *name, int& id)
{
    if (auto p = std::strstr(name, "[id:")) {
        if (sscanf(p, "[id:%08x]", &id) == 1) {
            return true;
        }
    }
    return false;
}

static inline float3 ToEular(const MQAngle& ang, bool flip_head = false)
{
    if (flip_head) {
        return float3{
            ang.pitch,
            -ang.head + 180.0f, // I can't explain why this modification is needed...
            ang.bank
        } * mu::Deg2Rad;
    }
    else {
        return float3{
            ang.pitch,
            ang.head,
            ang.bank
        } * mu::Deg2Rad;
    }
}

static inline quatf ToQuaternion(const MQAngle& ang)
{
    return rotateZXY(ToEular(ang));
}

static inline void ExtractLocalTransform(MQObject obj, float3& pos, quatf& rot, float3& scale)
{
    pos = to_float3(obj->GetTranslation());
    rot = ToQuaternion(obj->GetRotation());
    scale = to_float3(obj->GetScaling());
}

static inline float4x4 ExtractGlobalMatrix(MQDocument doc, MQObject obj)
{
    auto mat = to_float4x4(obj->GetLocalMatrix());
    if (auto parent = doc->GetParentObject(obj)) {
        auto pmat = ExtractGlobalMatrix(doc, parent);
        mat = mat * pmat;
    }
    return mat;
}
