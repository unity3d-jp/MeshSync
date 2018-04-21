#pragma  once

#define InchToMillimeter 25.4f

std::string GetName(MObject node);
std::string GetPath(MDagPath path);
std::string GetPath(MObject node);
std::string GetRootPath(MDagPath path);
std::string GetRootPath(MObject node);

MDagPath GetDagPath(MObject node);
bool IsVisible(MObject node);
MObject GetTransform(MDagPath path);
MObject GetTransform(MObject node);
MObject GetShape(MObject node);

MObject FindMesh(MObject node);
MObject FindSkinCluster(MObject node);
MObject FindBlendShape(MObject node);
MObject FindOrigMesh(MObject node);
MObject FindInputMesh(const MFnGeometryFilter& gf, const MDagPath& path);
MObject FindOutputMesh(const MFnGeometryFilter& gf, const MDagPath& path);

bool JointGetSegmentScaleCompensate(MObject joint);
bool JointGetInverseScale(MObject joint, mu::float3& dst);
float ToSeconds(MTime t);
MTime ToMTime(float seconds);

#ifdef mscDebug
    void DumpPlugInfoImpl(MPlug plug);
    #define DumpPlugInfo DumpPlugInfoImpl
#else
    #define DumpPlugInfo
#endif


template<class T> T* ptr(T& v) { return (T*)&(int&)v; }

bool GetAnimationCurve(MFnAnimCurve& dst, MPlug& src);
RawVector<float> BuildTimeSamples(const std::initializer_list<MFnAnimCurve*>& cvs, int samples_per_seconds);

void ConvertAnimationBool(
    RawVector<ms::TVP<bool>>& dst,
    bool default_value, MPlug& pb, int samples_per_seconds);

void ConvertAnimationFloat(
    RawVector<ms::TVP<float>>& dst,
    float default_value, MPlug& pb, int samples_per_seconds);

void ConvertAnimationFloat3(
    RawVector<ms::TVP<mu::float3>>& dst,
    const mu::float3& default_value, MPlug& px, MPlug& py, MPlug& pz, int samples_per_seconds);

void ConvertAnimationFloat4(
    RawVector<ms::TVP<mu::float4>>& dst,
    const mu::float4& default_value, MPlug& px, MPlug& py, MPlug& pz, MPlug& pw, int samples_per_seconds);


inline mu::float3 to_float3(const MPoint& v)
{
    return { (float)v.x, (float)v.y, (float)v.z };
}
inline mu::float3 to_float3(const MVector& v)
{
    return { (float)v.x, (float)v.y, (float)v.z };
}
inline mu::float3 to_float3(const MFloatPoint& v)
{
    return (const mu::float3&)v;
}
inline mu::float3 to_float3(const MFloatVector& v)
{
    return (const mu::float3&)v;
}
inline mu::float3 to_float3(const double (&v)[3])
{
    return { (float)v[0], (float)v[1], (float)v[2] };
}
inline mu::float4 to_float4(const MColor& v)
{
    return (const mu::float4&)v;
}
inline mu::quatf to_quatf(const MQuaternion& v)
{
    return { (float)v.x, (float)v.y, (float)v.z, (float)v.w };
}

