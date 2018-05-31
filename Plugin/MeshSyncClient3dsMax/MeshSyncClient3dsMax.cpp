#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxUtils.h"
#include "msmaxCallbacks.h"

#ifdef _WIN32
#pragma comment(lib, "core.lib")
#pragma comment(lib, "gup.lib")
#pragma comment(lib, "maxutil.lib")
#pragma comment(lib, "maxscrpt.lib")
#pragma comment(lib, "paramblk2.lib")
#endif



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
    RegisterNotification(OnStartup, this, NOTIFY_SYSTEM_STARTUP);
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
}

void MeshSyncClient3dsMax::onStartup()
{
    GetCOREInterface()->RegisterViewportDisplayCallback(TRUE, &msmaxViewportDisplayCallback::getInstance());
    GetCOREInterface()->RegisterTimeChangeCallback(&msmaxTimeChangeCallback::getInstance());
    RegisterNotification(OnNodeRenamed, this, NOTIFY_NODE_RENAMED);
    m_cbkey = GetISceneEventManager()->RegisterCallback(msmaxNodeCallback::getInstance().GetINodeEventCallback());
}

void MeshSyncClient3dsMax::onSceneUpdated()
{
    m_scene_updated = true;
}

void MeshSyncClient3dsMax::onTimeChanged()
{
    mscTraceW(L"MeshSyncClient3dsMax::onTimeChanged()\n");
}

void MeshSyncClient3dsMax::onNodeAdded(INode * n)
{
    m_scene_updated = true;
}

void MeshSyncClient3dsMax::onNodeDeleted(INode * n)
{
    m_scene_updated = true;

    m_deleted.push_back(GetPath(n));
    m_node_records.erase(n);
}

void MeshSyncClient3dsMax::onNodeUpdated(INode * n)
{
    mscTraceW(L"MeshSyncClient3dsMax::onNodeUpdated(): %s\n", n->GetName());
    m_node_records[n].dirty = true;
}

void MeshSyncClient3dsMax::onRepaint()
{
    mscTraceW(L"MeshSyncClient3dsMax::onRepaint()\n");
}


void MeshSyncClient3dsMax::update()
{
    if (m_pending_request != SendScope::None) {
        if (sendScene(m_pending_request)) {
            m_pending_request = SendScope::None;
        }
    }
    else if (m_settings.auto_sync && m_dirty) {
        sendScene(SendScope::Updated);
    }
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope)
{
    if (isSending()) {
        return false;
    }

    int num_exported = 0;
    if (scope == SendScope::All) {
        EnumerateAllNode([&](INode *n) {
            auto it = m_node_records.find(n);
            if (it != m_node_records.end())
                it->second.dirty = false;

            if(exportNode(n))
                ++num_exported;
        });
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_node_records) {
            auto& rec = kvp.second;
            if (rec.dirty) {
                rec.dirty = false;
                if (exportNode(kvp.first))
                    ++num_exported;
            }
        }
    }
    m_dirty = false;

    if (num_exported > 0)
        kickAsyncSend();
    return true;
}

bool MeshSyncClient3dsMax::sendAnimations(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::recvScene()
{
    return false;
}


bool MeshSyncClient3dsMax::isSending() const
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}

void MeshSyncClient3dsMax::waitSendComplete()
{
}

void MeshSyncClient3dsMax::kickAsyncSend()
{
}


ms::TransformPtr MeshSyncClient3dsMax::exportNode(INode * node)
{
    return ms::TransformPtr();
}

bool MeshSyncClient3dsMax::extractTransform(ms::Transform & dst, INode * src)
{
    return false;
}

bool MeshSyncClient3dsMax::extractCamera(ms::Camera & dst, INode * src)
{
    return false;
}

bool MeshSyncClient3dsMax::extractLight(ms::Light & dst, INode * src)
{
    return false;
}

bool MeshSyncClient3dsMax::extractMesh(ms::Mesh & dst, INode * src)
{
    return false;
}



def_visible_primitive(UnityMeshSync_ExportScene, "UnityMeshSync_ExportScene");
Value* UnityMeshSync_ExportScene_cf(Value** arg_list, int count)
{
    MeshSyncClient3dsMax::getInstance().sendScene(MeshSyncClient3dsMax::SendScope::All);
    return &ok;
}

def_visible_primitive(UnityMeshSync_ExportAnimations, "UnityMeshSync_ExportAnimations");
Value* UnityMeshSync_ExportAnimations_cf(Value** arg_list, int count)
{
    MeshSyncClient3dsMax::getInstance().sendAnimations(MeshSyncClient3dsMax::SendScope::All);
    return &ok;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        MaxSDK::Util::UseLanguagePackLocale();
        MeshSyncClient3dsMax::getInstance(); // initialize instance
        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    return(TRUE);
}

msmaxAPI const TCHAR* LibDescription()
{
    return _T("UnityMeshSync for 3ds Max (Release " msReleaseDateStr ") (Unity Technologies)");
}

msmaxAPI int LibNumberClasses()
{
    return 0;
}

msmaxAPI ClassDesc* LibClassDesc(int i)
{
    return nullptr;
}

msmaxAPI ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

msmaxAPI ULONG CanAutoDefer()
{
    return 0;
}
