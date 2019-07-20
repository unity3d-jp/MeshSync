#include "pch.h"
#include "msmaxUtils.h"

TimeValue GetTime()
{
    return GetCOREInterface()->GetTime();
}
float ToSeconds(TimeValue tics)
{
    return TicksToSec(tics);
}
TimeValue ToTicks(float sec)
{
    return SecToTicks(sec);
}

std::wstring GetNameW(INode *n)
{
    return n->GetName();
}
std::string GetName(INode *n)
{
    return mu::ToMBS(n->GetName());
}

std::wstring GetPathW(INode *n)
{
    std::wstring ret;
    auto parent = n->GetParentNode();
    if (parent && parent->GetObjectRef()) {
        ret = GetPathW(parent);
    }
    ret += L'/';
    ret += n->GetName();
    return ret;
}
std::string GetPath(INode *n)
{
    return mu::ToMBS(GetPathW(n));
}

mu::float4x4 GetPivotMatrix(INode *n)
{
    auto t = to_float3(n->GetObjOffsetPos());
    auto r = to_quatf(n->GetObjOffsetRot());
    return mu::transform(t, r, mu::float3::one());
}

mu::float4x4 GetTransform(INode *n, TimeValue t, bool bake_modifiers)
{
    if (bake_modifiers) {
        //auto *tm = n->EvalWorldState(t).GetTM();
        //return tm ? to_float4x4(*tm) : mu::float4x4::identity();
        return to_float4x4(n->GetObjTMAfterWSM(t));
    }
    else {
        return to_float4x4(n->GetNodeTM(t));
    }
}

bool IsVisibleInHierarchy(INode *n, TimeValue t)
{
    auto parent = n->GetParentNode();
    if (parent && !IsVisibleInHierarchy(parent, t))
        return false;
    return  n->GetVisibility(t) > 0.0f;
}

bool IsInWorldSpace(INode *n, TimeValue t)
{
    return n->GetObjTMAfterWSM(t).IsIdentity();
}

bool IsInstanced(INode *n)
{
    INodeTab instances;
    return IInstanceMgr::GetInstanceMgr()->GetInstances(*n, instances);
}

Object* GetTopObject(INode *n)
{
    return n->GetObjectRef();
}

Object* GetBaseObject(INode *n)
{
    return EachObject(n, [](Object*) {});
}

Modifier* FindSkin(INode *n)
{
    Modifier *ret = nullptr;
    EachModifier(n, [&ret](IDerivedObject *obj, Modifier *mod, int mi) {
        if (mod->ClassID() == SKIN_CLASSID && mod->IsEnabled() && !ret) {
            ret = mod;
        }
    });
    return ret;
}

ISkin* FindSkinInterface(INode *n)
{
    if (auto mod = FindSkin(n))
        return (ISkin*)mod->GetInterface(I_SKIN);
    return nullptr;
}

Modifier* FindMorph(INode *n)
{
    Modifier *ret = nullptr;
    EachModifier(n, [&ret](IDerivedObject *obj, Modifier *mod, int mi) {
        if (mod->ClassID() == MR3_CLASS_ID && mod->IsEnabled() && !ret) {
            ret = mod;
        }
    });
    return ret;
}

bool IsMesh(Object *obj)
{
    if (!obj)
        return false;
    return obj->SuperClassID() == GEOMOBJECT_CLASS_ID;
}

TriObject* GetSourceMesh(INode * n, bool& needs_delete)
{
    IDerivedObject *dobj = nullptr;
    int mod_index = 0;
    needs_delete = false;

    Modifier *skin_top = nullptr, *morph_top = nullptr;
    bool return_next = true;
    Object *base = EachModifier(n, [&](IDerivedObject *obj, Modifier *mod, int mi) {
        if (return_next) {
            return_next = false;
            dobj = obj;
            mod_index = mi;
        }

        if (mod->ClassID() == SKIN_CLASSID && !skin_top) {
            skin_top = mod;
            return_next = true;
        }
        else if (mod->ClassID() == MR3_CLASS_ID && !morph_top) {
            morph_top = mod;
            return_next = true;
        }
    });

    TriObject *ret = nullptr;
    auto to_triobject = [&needs_delete, &ret](Object *obj) {
        if (obj->CanConvertToType(triObjectClassID)) {
            auto old = obj;
            ret = (TriObject*)obj->ConvertToType(GetTime(), triObjectClassID);
            if (ret != old)
                needs_delete = true;
        }
    };

    if (return_next) {
        to_triobject(base);
    }
    else if (dobj) {
        auto os = dobj->Eval(GetTime(), mod_index);
        to_triobject(os.obj);
    }
    else {
        to_triobject(n->GetObjectRef());
    }
    return ret;
}

TriObject* GetFinalMesh(INode *n, bool &needs_delete)
{
    auto name = GetName(n);
    auto time = GetTime();
    auto cid = Class_ID(TRIOBJ_CLASS_ID, 0);
    auto ws = n->EvalWorldState(time);
    auto valid = ws.tmValid();
    auto *obj = ws.obj;
    if (obj->CanConvertToType(cid)) {
        auto *tri = (TriObject*)obj->ConvertToType(time, cid);
        if (obj != tri)
            needs_delete = true;
        return tri;
    }
    return nullptr;
}
