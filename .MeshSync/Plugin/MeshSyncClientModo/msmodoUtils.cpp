#include "pch.h"
#include "msmodoUtils.h"

CLxUser_Item GetParent(CLxUser_Item& obj)
{
    CLxUser_Item parent;
    if (valid(obj) && LXx_OK(obj.Parent(parent)))
        return parent;
    return nullptr;
}

std::string GetName(CLxUser_Item& obj)
{
    if (!valid(obj))
        return std::string();
    const char *name;
    obj.UniqueName(&name);
    return name ? name : std::string();
}

std::string GetPath(CLxUser_Item& obj)
{
    if (!valid(obj))
        return std::string();
    std::string ret;
    if (auto parent = GetParent(obj))
        ret += GetPath(parent);
    ret += '/';
    ret += GetName(obj);
    return ret;
}


class GetMapNames_Visitor : public CLxImpl_AbstractVisitor
{
public:
    GetMapNames_Visitor(CLxUser_MeshMap *mmap) : m_mmap(mmap) {}

    LxResult Evaluate() override
    {
        const char *name;
        if (LXx_OK(m_mmap->Name(&name)))
            m_names.push_back(name);
        return LXe_OK;
    }

    CLxUser_MeshMap *m_mmap;
    std::vector<const char*> m_names;
};

std::vector<const char*> GetMapNames(CLxUser_MeshMap& mmap, const LXtID4& id4)
{
    GetMapNames_Visitor name_visitor(&mmap);
    mmap.FilterByType(id4);
    mmap.Enum(&name_visitor);
    return name_visitor.m_names;
}
