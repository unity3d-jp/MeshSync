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


// Body: [](INode *node) -> void
template<class Body>
void EachNode(NodeEventNamespace::NodeKeyTab& nkt, const Body& body)
{
    int count = nkt.Count();
    for (int i = 0; i < count; ++i) {
        if (auto *n = NodeEventNamespace::GetNodeByKey(nkt[i])) {
            body(n);
        }
    }
}

static void DumpNodes(NodeEventNamespace::NodeKeyTab& nkt)
{
    EachNode(nkt, [](INode *node) {
        mscTraceW(L"node: %s\n", node->GetName());
    });
}

void msmaxNodeCallback::Added(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeAdded(n);
    });
}

void msmaxNodeCallback::Deleted(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}

void msmaxNodeCallback::LinkChanged(NodeKeyTab & nodes)
{
    MeshSyncClient3dsMax::getInstance().onSceneUpdated();
}

void msmaxNodeCallback::ModelStructured(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}
void msmaxNodeCallback::GeometryChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}
void msmaxNodeCallback::TopologyChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}
void msmaxNodeCallback::MappingChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}
void msmaxNodeCallback::ExtentionChannelChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}
void msmaxNodeCallback::ModelOtherEvent(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}

void msmaxTimeChangeCallback::TimeChanged(TimeValue t)
{
    MeshSyncClient3dsMax::getInstance().onTimeChanged();
}

static void OnStartup(void *param, NotifyInfo *info)
{
    ((MeshSyncClient3dsMax*)param)->onStartup();
}
static void OnNodeRenamed(void *param, NotifyInfo *info)
{
    ((MeshSyncClient3dsMax*)param)->onSceneUpdated();
}


MeshSyncClient3dsMax & MeshSyncClient3dsMax::getInstance()
{
    static MeshSyncClient3dsMax s_plugin;
    return s_plugin;
}

MeshSyncClient3dsMax::MeshSyncClient3dsMax()
{
    GetCOREInterface()->RegisterTimeChangeCallback(&g_msmaxTimeChangeCallback);
    RegisterNotification(OnStartup, this, NOTIFY_SYSTEM_STARTUP);
    RegisterNotification(OnNodeRenamed, this, NOTIFY_NODE_RENAMED);
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
}

void MeshSyncClient3dsMax::registerNodeCallback()
{
    if (m_cbkey == 0) {
        m_cbkey = GetISceneEventManager()->RegisterCallback(g_msmaxNodeCallback.GetINodeEventCallback());
    }
}

void MeshSyncClient3dsMax::onStartup()
{
    registerNodeCallback();
}

void MeshSyncClient3dsMax::onSceneUpdated()
{
    mscTraceW(L"MeshSyncClient3dsMax::onSceneUpdated()\n");
}

void MeshSyncClient3dsMax::onTimeChanged()
{
    mscTraceW(L"MeshSyncClient3dsMax::onTimeChanged()\n");
}

void MeshSyncClient3dsMax::onNodeAdded(INode * n)
{
    mscTraceW(L"MeshSyncClient3dsMax::onNodeAdded(): %s\n", n->GetName());
}

void MeshSyncClient3dsMax::onNodeDeleted(INode * n)
{
    mscTraceW(L"MeshSyncClient3dsMax::onNodeDeleted(): %s\n", n->GetName());
}

void MeshSyncClient3dsMax::onNodeUpdated(INode * n)
{
    mscTraceW(L"MeshSyncClient3dsMax::onNodeUpdated(): %s\n", n->GetName());
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
