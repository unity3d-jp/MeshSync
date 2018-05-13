#pragma  once

#define InchToMillimeter 25.4f

std::string GetName(MObject node);
std::string GetPath(MDagPath path);
std::string GetPath(MObject node);
std::string GetRootBonePath(MObject joint);

MUuid GetUUID(MObject node);
std::string GetUUIDString(MObject node);

MDagPath GetDagPath(MObject node);
bool IsVisible(MObject node);
MObject GetTransform(MDagPath path);
MObject GetTransform(MObject node);
MObject GetShape(MObject node);
MObject GetParent(MObject node);

MObject FindMesh(MObject node);
MObject FindSkinCluster(MObject node);
MObject FindBlendShape(MObject node);
MObject FindOrigMesh(MObject node);

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
inline mu::float4x4 to_float4x4(const MMatrix& v)
{
    return mu::float4x4{
        (float)v[0][0], (float)v[0][1], (float)v[0][2], (float)v[0][3],
        (float)v[1][0], (float)v[1][1], (float)v[1][2], (float)v[1][3],
        (float)v[2][0], (float)v[2][1], (float)v[2][2], (float)v[2][3],
        (float)v[3][0], (float)v[3][1], (float)v[3][2], (float)v[3][3],
    };
}
inline mu::quatf to_quatf(const MQuaternion& v)
{
    return { (float)v.x, (float)v.y, (float)v.z, (float)v.w };
}

inline bool to_bool(const MPlug plug)
{
    return plug.asBool();
}
inline mu::float3 to_float3(const MPlug plug)
{
    return mu::float3{
        (float)plug.child(0).asDouble(),
        (float)plug.child(1).asDouble(),
        (float)plug.child(2).asDouble()
    };
}



// body: [](MObject&) -> void
template<class Body>
void Enumerate(MFn::Type type, const Body& body)
{
    MItDag it(MItDag::kDepthFirst, type);
    while (!it.isDone()) {
        auto obj = it.item();
        body(obj);
        it.next();
    }
}

// body: [](MObject&) -> void
template<class Body>
inline void EachChild(MObject node, const Body& body)
{
    MFnDagNode fn(node);
    auto num_children = fn.childCount();
    for (uint32_t i = 0; i < num_children; ++i) {
        body(fn.child(i));
    }
}

// body: [](MObject&) -> void
template<class Body>
void EachConstraints(MObject node, const Body& body)
{
    MItDependencyGraph it(node, MFn::kConstraint, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        body(it.currentItem());
    }
}
