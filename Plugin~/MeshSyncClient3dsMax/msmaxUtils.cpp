#include "pch.h"
#include "msmaxUtils.h"
#include "msmaxContext.h"

TimeValue GetTime()
{
    return msmaxGetContext().getExportTime();
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
    std::wstring ret = n->GetName();
    // 3ds max allows name to contain '/'
    mu::SanitizeNodeName(ret);
    return ret;
}
std::string GetName(INode *n)
{
    return mu::ToMBS(GetNameW(n));
}

std::wstring GetPathW(INode *n)
{
    std::wstring ret;
    auto parent = n->GetParentNode();
    if (parent && parent->GetObjectRef())
        ret = GetPathW(parent);
    ret += L'/';
    ret += GetNameW(n);
    return ret;
}
std::string GetPath(INode *n)
{
    return mu::ToMBS(GetPathW(n));
}

std::string GetName(MtlBase *n)
{
    std::wstring ret = n->GetName().data();
    mu::SanitizeNodeName(ret);
    return mu::ToMBS(ret);
}

std::string GetCurrentMaxFileName()
{
    MSTR filename = ::GetCOREInterface()->GetCurFileName();
    auto len = filename.Length();
    if (len == 0)
        return "Untitled";
    else
        return mu::GetFilename_NoExtension(filename.data());
}

std::tuple<int, int> GetActiveFrameRange()
{
    auto time_range = ::GetCOREInterface()->GetAnimRange();
    auto tick_per_frame = ::GetTicksPerFrame();
    return {
        time_range.Start() / tick_per_frame,
        time_range.End() / tick_per_frame
    };
}

bool IsRenderable(INode *n)
{
    if (!n)
        return false;
    return n->Renderable() != 0;
}

bool VisibleInRender(INode *n, TimeValue t)
{
    auto parent = n->GetParentNode();
    if (parent && !VisibleInRender(parent, t))
        return false;
    return  n->GetVisibility(t) > 0.0f;
}

bool VisibleInViewport(INode *n)
{
    return n->IsHidden() == 0;
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

bool IsCamera(Object *obj)
{
    if (!obj)
        return false;
    auto cid = obj->SuperClassID();
    if (cid == CAMERA_CLASS_ID) {
        return true;
    }
    return false;
}

bool IsPhysicalCamera(Object *obj)
{
    return dynamic_cast<MaxSDK::IPhysicalCamera*>(obj) != nullptr;
}

bool IsLight(Object *obj)
{
    if (!obj)
        return false;
    auto cid = obj->SuperClassID();
    if (cid == LIGHT_CLASS_ID) {
        return true;
    }
    else if (obj->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS)) {
        return true;
    }
    return false;
}

bool IsMesh(Object *obj)
{
    if (!obj)
        return false;
    return obj->SuperClassID() == GEOMOBJECT_CLASS_ID ||
        obj->CanConvertToType(triObjectClassID);
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

TriObject* GetFinalMesh(INode *n, bool& needs_delete)
{
    auto time = GetTime();
    auto cid = Class_ID(TRIOBJ_CLASS_ID, 0);
    auto& ws = n->EvalWorldState(time);
    auto *obj = ws.obj;
    if (obj->CanConvertToType(cid)) {
        auto *tri = (TriObject*)obj->ConvertToType(time, cid);
        if (obj != tri)
            needs_delete = true;
        return tri;
    }
    return nullptr;
}


int RenderScope::RenderBeginProc::proc(ReferenceMaker* rm)
{
    rm->RenderBegin(time);
    return REF_ENUM_CONTINUE;
}

int RenderScope::RenderEndProc::proc(ReferenceMaker* rm)
{
    rm->RenderEnd(time);
    return REF_ENUM_CONTINUE;
}

void RenderScope::prepare(TimeValue t)
{
    m_beginp.time = t;
    m_endp.time = t;
}

void RenderScope::addNode(INode *n)
{
    m_nodes.push_back(n);
}

void RenderScope::begin()
{
    m_beginp.BeginEnumeration();
    for (auto n : m_nodes)
        n->EnumRefHierarchy(m_beginp);
    m_beginp.EndEnumeration();
}

void RenderScope::end()
{
    m_endp.BeginEnumeration();
    for (auto n : m_nodes)
        n->EnumRefHierarchy(m_endp);
    m_endp.EndEnumeration();

    m_nodes.clear();
}


NullView::NullView()
{
    worldToView.IdentityMatrix();
    screenW = 640.0f; screenH = 480.0f;
}

Point2 NullView::ViewToScreen(Point3 p)
{
    return Point2(p.x, p.y);
}
