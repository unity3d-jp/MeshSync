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
        if (mod->ClassID() == SKIN_CLASSID && !ret) {
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
        if (mod->ClassID() == MR3_CLASS_ID && !ret) {
            ret = mod;
        }
    });
    return ret;
}

bool IsBone(Object *obj)
{
    if (!obj)
        return false;

    // not sure this is correct...
    auto cid = obj->ClassID();
    return
        cid == Class_ID(BONE_CLASS_ID, 0) ||
        cid == BONE_OBJ_CLASSID ||
        cid == SKELOBJ_CLASS_ID
#if MAX_PRODUCT_YEAR_NUMBER >= 2018
        ||
        cid == CATBONE_CLASS_ID ||
        cid == CATHUB_CLASS_ID
#endif
        ;
}

bool IsMesh(Object *obj)
{
    if (!obj)
        return false;
    return obj->SuperClassID() == GEOMOBJECT_CLASS_ID && !IsBone(obj);
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

TriObject* GetFinalMesh(INode * n, bool & needs_delete)
{
    IDerivedObject *dobj = nullptr;
    int mod_index = 0;
    needs_delete = false;

    Object *base = EachModifier(n, [&](IDerivedObject *obj, Modifier *mod, int mi) {
        if (!dobj) {
            dobj = obj;
            mod_index = mi;
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

    if (dobj) {
        auto os = dobj->Eval(GetTime(), mod_index);
        to_triobject(os.obj);
    }
    else {
        to_triobject(base);
    }
    return ret;
}
