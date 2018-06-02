#include "pch.h"
#include "msmaxUtils.h"

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
    if (auto parent = n->GetParentNode())
        ret = GetPathW(parent);
    ret += L'/';
    ret += n->GetName();
    return ret;
}

std::string GetPath(INode *n)
{
    return mu::ToMBS(GetPathW(n));
}

Object * GetBaseObject(INode * n)
{
    return EachObject(n, [](Object*) {});
}

ISkin* FindSkin(INode *n)
{
    ISkin *ret = nullptr;
    EachModifier(n, [&ret](Object *obj, Modifier *mod) {
        if (mod->ClassID() == SKIN_CLASSID) {
            ret = (ISkin*)mod->GetInterface(I_SKIN);
        }
    });
    return ret;
}

Modifier* FindMorph(INode * n)
{
    Modifier *ret = nullptr;
    EachModifier(n, [&ret](Object *obj, Modifier *mod) {
        if (mod->ClassID() == MR3_CLASS_ID) {
            ret = mod;
        }
    });
    return ret;
}

