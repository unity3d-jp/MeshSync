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

bool IsMesh(Object *obj)
{
    return obj && obj->SuperClassID() == GEOMOBJECT_CLASS_ID && obj->ClassID() != BONE_OBJ_CLASSID;
}

TriObject* GetSourceMesh(INode * n)
{
    IDerivedObject *dobj = nullptr;
    int mod_index = 0;

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

    if (return_next) {
        if (base->CanConvertToType(triObjectClassID))
            return (TriObject*)base->ConvertToType(GetTime(), triObjectClassID);
    }
    else if (dobj) {
        auto os = dobj->Eval(GetTime(), mod_index);
        if (os.obj->CanConvertToType(triObjectClassID))
            return (TriObject*)os.obj->ConvertToType(GetTime(), triObjectClassID);
    }
    else {
        auto obj = n->GetObjectRef();
        if (obj->CanConvertToType(triObjectClassID))
            return (TriObject*)obj->ConvertToType(GetTime(), triObjectClassID);
    }
    return nullptr;
}
