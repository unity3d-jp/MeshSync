#include "pch.h"
#include "MeshSyncClient3dsMax.h"

#ifdef _WIN32
#pragma comment(lib, "core.lib")
#pragma comment(lib, "gup.lib")
#pragma comment(lib, "maxutil.lib")
#pragma comment(lib, "maxscrpt.lib")
#pragma comment(lib, "paramblk2.lib")
#endif

class msmaxNodeCallback : public INodeEventCallback
{
public:
    void Added(NodeKeyTab& nodes) override;
    void Deleted(NodeKeyTab& nodes) override;
    void LinkChanged(NodeKeyTab& nodes) override;

    void ModelStructured(NodeKeyTab& nodes) override;
    void GeometryChanged(NodeKeyTab& nodes) override;
    void TopologyChanged(NodeKeyTab& nodes) override;
    void MappingChanged(NodeKeyTab& nodes) override;
    void ExtentionChannelChanged(NodeKeyTab& nodes) override;
    void ModelOtherEvent(NodeKeyTab& nodes) override;

} g_msmaxNodeCallback;

class msmaxTimeChangeCallback : public TimeChangeCallback
{
public:
    void TimeChanged(TimeValue t) override;
} g_msmaxTimeChangeCallback;


static void DumpNodes(NodeEventNamespace::NodeKeyTab& nkt)
{
    int count = nkt.Count();
    for (int i = 0; i < count; ++i) {
        auto node = NodeEventNamespace::GetNodeByKey(nkt[i]);
        if (node) {
            mscTraceW(L"node: %s\n", node->GetName());
        }
    }
}

void msmaxNodeCallback::Added(NodeKeyTab & nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::Added()\n");
}

void msmaxNodeCallback::Deleted(NodeKeyTab & nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::Deleted()\n");
}

void msmaxNodeCallback::LinkChanged(NodeKeyTab & nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::LinkChanged()\n");
}

void msmaxTimeChangeCallback::TimeChanged(TimeValue t)
{
    mscTrace("msmaxTimeChangeCallback::TimeChanged()\n");
}

void msmaxNodeCallback::ModelStructured(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::ModelStructured()\n");
}
void msmaxNodeCallback::GeometryChanged(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::GeometryChanged()\n");
}
void msmaxNodeCallback::TopologyChanged(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::TopologyChanged()\n");
}
void msmaxNodeCallback::MappingChanged(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::MappingChanged()\n");
}
void msmaxNodeCallback::ExtentionChannelChanged(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::ExtentionChannelChanged()\n");
}
void msmaxNodeCallback::ModelOtherEvent(NodeKeyTab& nodes)
{
    DumpNodes(nodes);
    mscTrace("msmaxNodeCallback::ModelOtherEvent()\n");
}



static std::unique_ptr<MeshSyncClient3dsMax> g_plugin;

MeshSyncClient3dsMax & MeshSyncClient3dsMax::getInstance()
{
    return *g_plugin;
}

MeshSyncClient3dsMax::MeshSyncClient3dsMax()
{
    GetCOREInterface()->RegisterTimeChangeCallback(&g_msmaxTimeChangeCallback);
    m_cbkey = GetISceneEventManager()->RegisterCallback(g_msmaxNodeCallback.GetINodeEventCallback());
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
}

void MeshSyncClient3dsMax::update()
{
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::sendAnimations(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::recvScene()
{
    return false;
}


class msmaxNodeCallbackDesc :public ClassDesc2 {
public:
    int             IsPublic() { return 1; }
    void*           Create(BOOL) { return new msmaxNodeCallback(); }
    const TCHAR*    ClassName() { return _T("msmaxNodeCallback"); }
    SClass_ID       SuperClassID() { return GUP_CLASS_ID; }
    Class_ID        ClassID() { return Class_ID(0xdd174b35, 0x9fc48134); }
    const TCHAR*    Category() { return _T("Global Utility PlugIn"); }
} g_msmaxNodeCallbackDesc;



def_visible_primitive(UnityMeshSync_Export, "UnityMeshSync_Export");
Value* UnityMeshSync_Export_cf(Value** arg_list, int count)
{
    mscTrace("UnityMeshSync_Export_cf()\n");
    return &ok;
}


static HINSTANCE g_hinstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_hinstance = hinstDLL;
        MaxSDK::Util::UseLanguagePackLocale();
        g_plugin.reset(new MeshSyncClient3dsMax());
        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    return(TRUE);
}

msmaxAPI const TCHAR* LibDescription()
{
    return _T("UnityMeshSync for 3ds Max (Release " msReleaseDateStr ") (Unity Technologies)");
}

static ClassDesc *g_classdescs[] = {
    nullptr,
};
msmaxAPI int LibNumberClasses()
{
    return _countof(g_classdescs);
}

msmaxAPI ClassDesc* LibClassDesc(int i)
{
    if (i < _countof(g_classdescs)) {
        return g_classdescs[i];
    }
    return nullptr;
}

msmaxAPI ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

msmaxAPI ULONG CanAutoDefer()
{
    return 1;
}
