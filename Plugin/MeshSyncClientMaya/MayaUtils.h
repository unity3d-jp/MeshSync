#pragma  once

#define InchToMillimeter 25.4f

std::string GetName(const MObject& node);
std::string GetPath(const MDagPath& path);
std::string GetPath(const MObject& node);
std::string GetRootBonePath(const MObject& joint);

MDagPath GetDagPath(const MObject& node);
bool IsVisible(const MObject& node);
MObject GetTransform(const MDagPath& path);
MObject GetTransform(const MObject& node);
MObject GetShape(const MDagPath& node);
MObject GetShape(const MObject& node);
MDagPath GetParent(const MDagPath& node);
MObject GetParent(const MObject& node);
bool IsInstance(const MObject& node);

MObject FindMesh(const MObject& node);
MObject FindSkinCluster(const MObject& node);
MObject FindBlendShape(const MObject& node);
MObject FindOrigMesh(const MObject& node);

float ToSeconds(MTime t);
MTime ToMTime(float seconds);

#ifdef mscDebug
    void DumpPlugInfoImpl(MPlug plug);
    #define DumpPlugInfo DumpPlugInfoImpl
#else
    #define DumpPlugInfo
#endif


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
inline void EnumerateNode(MFn::Type type, const Body& body)
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
inline void EachParent(MObject node, const Body& body)
{
    MFnDagNode fn(node);
    auto num_parents = fn.parentCount();
    for (uint32_t i = 0; i < num_parents; ++i)
        body(fn.parent(i));
}

// body: [](MObject&) -> void
template<class Body>
inline void EachChild(MObject node, const Body& body)
{
    MFnDagNode fn(node);
    auto num_children = fn.childCount();
    for (uint32_t i = 0; i < num_children; ++i)
        body(fn.child(i));
}

// body: [](MObject&) -> void
template<class Body>
inline void EachConstraints(MObject node, const Body& body)
{
    MItDependencyGraph it(node, MFn::kConstraint, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        body(it.currentItem());
        it.next();
    }
}
