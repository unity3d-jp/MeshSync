#pragma once

#include "MeshSync/MeshSync.h"

TimeValue GetTime();
float ToSeconds(TimeValue tics);
TimeValue ToTicks(float sec);

std::wstring GetNameW(INode *n);
std::string  GetName(INode *n);
std::wstring GetPathW(INode *n);
std::string  GetPath(INode *n);
Object* GetTopObject(INode *n);
Object* GetBaseObject(INode *n);
Modifier* FindSkin(INode *n);
ISkin* FindSkinInterface(INode *n);
Modifier* FindMorph(INode * n);
bool IsMesh(Object *obj);
std::tuple<IDerivedObject*, int> GetSourceMesh(INode *n);


inline mu::float2 to_float2(const Point3& v)
{
    return { v.x, v.y };
}
inline mu::float3 to_float3(const Point3& v)
{
    return { v.x, v.y, v.z };
}
inline mu::float4 to_color(const Point3& v)
{
    return { v.x, v.y, v.z, 1.0f };
}

inline mu::float4x4 to_float4x4(const Matrix3& v)
{
    const float *f = (const float*)&v[0];
    return { {
        f[ 0], f[ 1], f[ 2], 0.0f,
        f[ 3], f[ 4], f[ 5], 0.0f,
        f[ 6], f[ 7], f[ 8], 0.0f,
        f[ 9], f[10], f[11], 1.0f
    } };
}



// Body: [](INode *node) -> void
template<class Body>
inline void EachNode(NodeEventNamespace::NodeKeyTab& nkt, const Body& body)
{
    int count = nkt.Count();
    for (int i = 0; i < count; ++i) {
        if (auto *n = NodeEventNamespace::GetNodeByKey(nkt[i])) {
            body(n);
        }
    }
}

// Body: [](IDerivedObject *obj) -> void
// return bottom object
template<class Body>
inline Object* EachObject(INode *n, const Body& body)
{
    Object* obj = n->GetObjectRef();
    while (obj) {
        body(obj);
        if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
            obj = ((IDerivedObject*)obj)->GetObjRef();
        else
            break;
    }
    return obj;
}

// Body: [](IDerivedObject *obj, Modifier *mod, int mod_index) -> void
template<class Body>
inline void EachModifier(INode *n, const Body& body)
{
    Object* obj = n->GetObjectRef();
    while (obj) {
        if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
            auto dobj = (IDerivedObject*)obj;
            int num_mod = dobj->NumModifiers();
            for (int mi = 0; mi < num_mod; ++mi)
                body(dobj, dobj->GetModifier(mi), mi);
            obj = dobj->GetObjRef();
        }
        else
            break;
    }
}

// Body: [](IDerivedObject *obj, Modifier *mod, int mod_index) -> void
template<class Body>
inline void EachModifier(Object *obj, const Body& body)
{
    if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
        auto dobj = (IDerivedObject*)obj;
        int num_mod = dobj->NumModifiers();
        for (int mi = 0; mi < num_mod; ++mi)
            body(obj, dobj->GetModifier(mi), mi);
    }
}



namespace detail {

    template<class Body>
    class EnumerateAllNodeImpl : public ITreeEnumProc
    {
    public:
        const Body & body;
        int ret;

        EnumerateAllNodeImpl(const Body& b, bool ignore_childrern = false)
            : body(b)
            , ret(ignore_childrern ? TREE_IGNORECHILDREN : TREE_CONTINUE)
        {}

        int callback(INode *node) override
        {
            body(node);
            return ret;
        }
    };

} // namespace detail

// Body: [](INode *node) -> void
template<class Body>
inline void EnumerateAllNode(const Body& body)
{
    if (auto *scene = GetCOREInterface7()->GetScene()) {
        detail::EnumerateAllNodeImpl<Body> cb(body);
        scene->EnumTree(&cb);
    }
    else {
        mscTrace("EnumerateAllNode() failed!!!\n");
    }
}

// Body : [](INode *bone) -> void
template<class Body>
inline void EachBone(ISkin *skin, const Body& body)
{
    int num_bones = skin->GetNumBones();
    for (int bi = 0; bi < num_bones; ++bi)
        body(skin->GetBone(bi));
}

// Body : [](INode *bone) -> void
template<class Body>
inline void EachBone(INode *n, const Body& body)
{
    if (auto *skin = FindSkinInterface(n))
        EachBone(skin, body);
}


#ifdef mscDebug
inline void DbgPrintNode(INode *node)
{
    mscTraceW(L"node: %s\n", node->GetName());
}
#else
#define DbgPrintNode(...)
#endif
