#include "pch.h"
#include "msmodoUtils.h"

std::string GetName(CLxUser_Item& obj)
{
    CLxUser_Item item(obj);
    const char *name;
    item.Name(&name);
    return name;
}

std::string GetPath(CLxUser_Item& obj)
{
    std::string ret;
    CLxUser_Item parent;
    if (obj.Parent(parent))
        ret += GetPath(parent);

    ret += '/';
    ret += GetName(obj);
    return ret;
}

