#pragma once

// for Modo 10

class CLxLoc_MeshFilter1 : public CLxLocalize<ILxMeshFilter1ID>
{
public:
    void _init() { m_loc = 0; }
    CLxLoc_MeshFilter1() { _init(); }
    CLxLoc_MeshFilter1(ILxUnknownID obj) { _init(); set(obj); }
    CLxLoc_MeshFilter1(const CLxLoc_MeshFilter1 &other) { _init(); set(other.m_loc); }
    const LXtGUID * guid() const { return &lx::guid_MeshFilter1; }
    unsigned
        Type(void)
    {
        return m_loc[0]->Type(m_loc);
    }

    LxResult
        Generate(void **ppvObj)
    {
        return m_loc[0]->Generate(m_loc, ppvObj);
    }

    CLxResult
        Generate(CLxLocalizedObject &o_dest)
    {
        LXtObjectID o_obj;
        LxResult r_c = m_loc[0]->Generate(m_loc, &o_obj);
        return lx::TakeResult(o_dest, r_c, o_obj);
    }
};

class CLxUser_MeshFilter1 : public CLxLoc_MeshFilter1
{
public:
    CLxUser_MeshFilter1() {}
    CLxUser_MeshFilter1(ILxUnknownID obj) : CLxLoc_MeshFilter1(obj) {}

    bool
        GetMesh(
            CLxLoc_Mesh             &mesh)
    {
        return Generate(mesh);
    }
};


