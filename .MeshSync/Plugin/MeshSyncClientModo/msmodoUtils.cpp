#include "pch.h"
#include "msmodoUtils.h"

CLxUser_Item GetParent(CLxUser_Item& obj)
{
    CLxUser_Item parent;
    if (obj.Parent(parent))
        return parent;
    return nullptr;
}

std::string GetName(CLxUser_Item& obj)
{
    const char *name;
    //obj.Name(&name);
    obj.UniqueName(&name);
    return name ? name : "";
}

std::string GetPath(CLxUser_Item& obj)
{
    std::string ret;
    if (auto parent = GetParent(obj))
        ret += GetPath(parent);
    ret += '/';
    ret += GetName(obj);
    return ret;
}

