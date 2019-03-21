#pragma  once

#define InchToMillimeter 25.4f

std::string GetName(const MObject& node);
std::string ToString(const MDagPath& path);
std::string RemoveNamespace(const std::string& path);

bool IsVisible(const MObject& node);
MDagPath GetParent(const MDagPath& node);
bool IsInstance(const MObject& node);

MObject FindSkinCluster(MObject node);
MObject FindBlendShape(MObject node);
MObject FindOrigMesh(const MObject& node);

float ToSeconds(MTime t);
MTime ToMTime(float seconds);

#ifdef mscDebug
    void DumpPlugInfoImpl(MPlug plug);
    void DumpPlugInfoImpl(MFnDependencyNode& fn);
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

inline bool to_bool(const MPlug& plug)
{
    return plug.asBool();
}
inline mu::float3 to_float3(const MPlug& plug)
{
    return mu::float3{
        (float)plug.child(0).asDouble(),
        (float)plug.child(1).asDouble(),
        (float)plug.child(2).asDouble()
    };
}


template<class T, size_t pad_size = 64>
struct Pad : public T
{
#if MAYA_LT
    Pad() {}

    template <typename... Args>
    Pad(Args&&... args) : T(std::forward<Args>(args)...) {}

    uint8_t padding[pad_size];

#else
    Pad() {}

    template <typename... Args>
    Pad(Args&&... args) : T(std::forward<Args>(args)...) {}

#endif
};


// body: [](MObject&) -> void
template<class Body>
inline void EnumerateNode(MFn::Type type, const Body& body)
{
    MItDag it(MItDag::kDepthFirst, type);
    while (!it.isDone()) {
        auto obj = it.currentItem();
        body(obj);
        it.next();
    }
}

// body: [](MObject&) -> void
template<class Body>
inline void EnumerateAllNode(const Body& body)
{
    MItDag it(MItDag::kDepthFirst, MFn::kInvalid);
    while (!it.isDone()) {
        auto obj = it.item();
        body(obj);
        it.next();
    }
}


// body: [](MObject&) -> void
template<class Body>
inline void EachParent(const MObject& node, const Body& body)
{
    Pad<MFnDagNode> fn(node);
    auto num_parents = fn.parentCount();
    for (uint32_t i = 0; i < num_parents; ++i)
        body(fn.parent(i));
}
// body: [](MObject&) -> void
template<class Body>
inline void EachParentRecursive(const MObject& node, const Body& body)
{
    Pad<MFnDagNode> fn(node);
    auto num_parents = fn.parentCount();
    for (uint32_t i = 0; i < num_parents; ++i) {
        auto parent = fn.parent(i);
        body(parent);
        EachParentRecursive(parent, body);
    }
}

// body: [](MObject&) -> void
template<class Body>
inline void EachChild(const MObject& node, const Body& body)
{
    Pad<MFnDagNode> fn(node);
    auto num_children = fn.childCount();
    for (uint32_t i = 0; i < num_children; ++i)
        body(fn.child(i));
}

// body: [](MObject&) -> void
template<class Body>
inline void EachConstraints(MObject& node, const Body& body)
{
    MItDependencyGraph it(node, MFn::kConstraint, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        body(it.currentItem());
        it.next();
    }
}

#ifdef mscDebug
    inline void PrintNodeInfo(MObject node, int depth = 0)
    {
        char indent[64];
        int d = 0;
        for (; d < depth; ++d) {
            indent[d] = ' ';
        }
        indent[d] = '\0';

        mscTrace("%s%s %d\n", indent, node.apiTypeStr(), node.apiType());
        for (MItDependencyGraph iter(node); !iter.isDone(); iter.next()) {
            if (iter.currentItem() != node) {
                auto item = iter.currentItem();
                mscTrace("%s* %s %d\n", indent, item.apiTypeStr(), item.apiType());
            }
        }

        EachChild(node, [&](const MObject& child) {
            PrintNodeInfo(child, depth + 1);
        });
    }
#else
    #define PrintNodeInfo(...)
#endif

