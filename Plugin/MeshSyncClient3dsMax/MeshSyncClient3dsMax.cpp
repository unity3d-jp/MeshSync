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

            if (exportNode(n))
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


void ExtractTransform(INode * node, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale)
{
    auto mat = to_float4x4(node->GetObjTMAfterWSM(t));
    if (auto parent = node->GetParentNode()) {
        auto pmat = to_float4x4(parent->GetObjTMAfterWSM(t));
        mat *= mu::invert(pmat);
    }
    mat = to_lhs(mat);
    pos = mu::extract_position(mat);
    rot = mu::extract_rotation(mat);
    scale = mu::extract_scale(mat);
}


ms::TransformPtr MeshSyncClient3dsMax::exportNode(INode * node)
{
    ms::TransformPtr ret;

    auto obj = node->GetObjectRef();
    if (obj->CanConvertToType(triObjectClassID)) {

    }

    if (!ret) {
        switch (node->SuperClassID()) {
        case CAMERA_CLASS_ID:
            if (m_settings.sync_cameras) {
                auto dst = ms::Camera::create();
                ret = dst;
                extractCamera(*dst, node);
            }
            break;
        case LIGHT_CLASS_ID:
            if (m_settings.sync_lights) {
                auto dst = ms::Light::create();
                ret = dst;
                extractLight(*dst, node);
            }
            break;
        default:
            if (m_settings.sync_meshes) {
                auto dst = ms::Transform::create();
                ret = dst;
                extractTransform(*dst, node);
            }
            break;
        }
    }
    return ret;
}

bool MeshSyncClient3dsMax::extractTransform(ms::Transform & dst, INode * src)
{
    ExtractTransform(src, GetTime(), dst.position, dst.rotation, dst.scale);
    return true;
}

bool MeshSyncClient3dsMax::extractCamera(ms::Camera & dst, INode * src)
{
    extractTransform(dst, src);
    return true;
}

bool MeshSyncClient3dsMax::extractLight(ms::Light & dst, INode * src)
{
    extractTransform(dst, src);
    return true;
}

bool MeshSyncClient3dsMax::extractMesh(ms::Mesh & dst, INode * src)
{
    extractTransform(dst, src);

    auto obj = src->GetObjectRef();
    auto tri = (TriObject*)obj->ConvertToType(GetTime(), triObjectClassID);
    auto& mesh = tri->GetMesh();

    // faces
    int num_faces = mesh.numFaces;
    int num_indices = num_faces * 3; // Max's Face is triangle
    {
        dst.counts.clear();
        dst.counts.resize(num_faces, 3);
        dst.material_ids.resize_discard(num_faces);

        const auto *faces = mesh.faces;
        dst.indices.resize_discard(num_indices);
        for (int fi = 0; fi < num_faces; ++fi) {
            auto& face = faces[fi];
            dst.material_ids[fi] = const_cast<Face&>(face).getMatID(); // :(
            for (int i = 0; i < 3; ++i)
                dst.indices[fi * 3 + i] = face.v[i];
        }
    }

    // points
    int num_vertices = mesh.numVerts;
    dst.points.resize_discard(num_vertices);
    dst.points.assign((mu::float3*)mesh.verts, (mu::float3*)mesh.verts + num_vertices);
    for (auto& v : dst.points) {
        v = to_lhs(v);
    }

    // uv
    if (m_settings.sync_uvs) {
        int num_uv = mesh.numTVerts;
        auto *uv_faces = mesh.tvFace;
        auto *uv_vertices = mesh.tVerts;
        if (num_uv && uv_faces && uv_vertices) {
            dst.uv0.resize_discard(num_indices);
            for (int fi = 0; fi < num_faces; ++fi) {
                for (int i = 0; i < 3; ++i) {
                    dst.uv0[fi * 3 + i] = to_float2(uv_vertices[uv_faces[fi].t[i]]);
                }
            }
        }
    }

    // colors
    if (m_settings.sync_colors) {
        int num_colors = mesh.numCVerts;
        auto *vc_faces = mesh.vcFace;
        auto *vc_vertices = mesh.vertCol;
        if (num_colors && vc_faces && vc_vertices) {
            dst.colors.resize_discard(num_indices);
            for (int fi = 0; fi < num_faces; ++fi) {
                for (int i = 0; i < 3; ++i) {
                    dst.colors[fi * 3 + i] = to_color(vc_vertices[vc_faces[fi].t[i]]);
                }
            }
        }
    }

    dst.setupFlags();
    return true;
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
