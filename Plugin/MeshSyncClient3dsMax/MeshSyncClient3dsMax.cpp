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
    waitAsyncSend();
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

            if (exportObject(n))
                ++num_exported;
        });
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_node_records) {
            auto& rec = kvp.second;
            if (rec.dirty) {
                rec.dirty = false;
                if (exportObject(kvp.first))
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
    if (isSending()) {
        waitAsyncSend();
    }

    // create default clip
    m_animations.push_back(ms::AnimationClip::create());

    // gather target data
    int num_exported = 0;
    if (scope == SendScope::All) {
        EnumerateAllNode([&](INode *n) {
            if (exportAnimations(n))
                ++num_exported;
        });
    }
    else {
        // todo:
    }

    // advance frame and record animation
    auto time_range = GetCOREInterface()->GetAnimRange();
    auto interval = SecToTicks(1.0f / m_settings.animation_sps);
    for (TimeValue t = time_range.Start(); t <= time_range.End(); t += interval) {
        // advance frame and record
        m_current_time = t;
        for (auto& kvp : m_anim_records)
            kvp.second(this);
    }

    // cleanup intermediate data
    m_anim_records.clear();

    // keyframe reduction
    for (auto& clip : m_animations)
        clip->reduction();

    // erase empty animation
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
        m_animations.end());

    if (num_exported > 0)
        kickAsyncSend();
    else
        m_animations.clear();
    return true;
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

void MeshSyncClient3dsMax::waitAsyncSend()
{
    if (m_future_send.valid()) {
        m_future_send.wait_for(std::chrono::milliseconds(m_settings.timeout_ms));
    }
}

void MeshSyncClient3dsMax::kickAsyncSend()
{
    for (auto& kvp : m_node_records)
        kvp.second.clearState();

    // begin async send
    m_future_send = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = m_settings.scale_factor;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send delete message
        size_t num_deleted = m_deleted.size();
        if (num_deleted) {
            ms::DeleteMessage del;
            del.targets.resize(num_deleted);
            for (uint32_t i = 0; i < num_deleted; ++i)
                del.targets[i].path = m_deleted[i];
            m_deleted.clear();

            client.send(del);
        }

        // send scene data
        {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.objects = m_objects;
            set.scene.materials = m_materials;
            client.send(set);

            m_objects.clear();
            m_materials.clear();
        }

        // send meshes one by one to Unity can respond quickly
        for (auto& mesh : m_meshes) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.objects = { mesh };
            client.send(set);
        };
        m_meshes.clear();

        // send animations and constraints
        if (!m_animations.empty() || !m_constraints.empty()) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.animations = m_animations;
            set.scene.constraints = m_constraints;
            client.send(set);

            m_animations.clear();
            m_constraints.clear();
        }

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}


static void ExtractTransform(INode * node, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale)
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


ms::TransformPtr MeshSyncClient3dsMax::exportObject(INode * n)
{
    ms::TransformPtr ret;

    auto obj = n->GetObjectRef();
    if (obj->CanConvertToType(triObjectClassID)) {
        auto dst = ms::Mesh::create();
        ret = dst;
        m_meshes.push_back(dst);
        extractMeshData(*dst, n);
    }

    if (!ret) {
        switch (n->SuperClassID()) {
        case CAMERA_CLASS_ID:
            if (m_settings.sync_cameras) {
                auto dst = ms::Camera::create();
                ret = dst;
                m_objects.push_back(dst);
                extractCameraData(*dst, n);
            }
            break;
        case LIGHT_CLASS_ID:
            if (m_settings.sync_lights) {
                auto dst = ms::Light::create();
                ret = dst;
                m_objects.push_back(dst);
                extractLightData(*dst, n);
            }
            break;
        default:
            if (m_settings.sync_meshes) {
                auto dst = ms::Transform::create();
                ret = dst;
                m_objects.push_back(dst);
                extractTransformData(*dst, n);
            }
            break;
        }
    }

    if (ret) {
        ret->path = GetPath(n);
        ret->index = ++m_index_seed;
        m_node_records[n].dst_obj = ret.get();
    }
    return ret;
}

bool MeshSyncClient3dsMax::extractTransformData(ms::Transform & dst, INode * src)
{
    ExtractTransform(src, GetTime(), dst.position, dst.rotation, dst.scale);
    return true;
}

bool MeshSyncClient3dsMax::extractCameraData(ms::Camera & dst, INode * src)
{
    extractTransformData(dst, src);
    return true;
}

bool MeshSyncClient3dsMax::extractLightData(ms::Light & dst, INode * src)
{
    extractTransformData(dst, src);
    return true;
}

bool MeshSyncClient3dsMax::extractMeshData(ms::Mesh & dst, INode * src)
{
    extractTransformData(dst, src);

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

ms::AnimationPtr MeshSyncClient3dsMax::exportAnimations(INode * n)
{
    auto it = m_anim_records.find(n);
    if (it != m_anim_records.end())
        return ms::AnimationPtr();

    auto& animations = m_animations[0]->animations;
    ms::AnimationPtr ret;
    AnimationRecord::extractor_t extractor = nullptr;

    auto obj = n->GetObjectRef();
    if (obj->CanConvertToType(triObjectClassID)) {
        auto dst = ms::MeshAnimation::create();
        animations.push_back(dst);
        ret = dst;
        extractor = &MeshSyncClient3dsMax::extractMeshAnimation;
    }

    if (!ret) {
        switch (n->SuperClassID()) {
        case CAMERA_CLASS_ID:
            if (m_settings.sync_cameras) {
                auto dst = ms::CameraAnimation::create();
                animations.push_back(dst);
                ret = dst;
                extractor = &MeshSyncClient3dsMax::extractCameraAnimation;
            }
            break;
        case LIGHT_CLASS_ID:
            if (m_settings.sync_lights) {
                auto dst = ms::LightAnimation::create();
                animations.push_back(dst);
                ret = dst;
                extractor = &MeshSyncClient3dsMax::extractLightAnimation;
            }
            break;
        default:
            if (m_settings.sync_meshes) {
                auto dst = ms::TransformAnimation::create();
                animations.push_back(dst);
                ret = dst;
                extractor = &MeshSyncClient3dsMax::extractTransformAnimation;
            }
            break;
        }
    }

    if (ret) {
        ret->path = GetPath(n);

        auto& rec = m_anim_records[n];
        rec.dst = ret.get();
        rec.src = n;
        rec.extractor = extractor;
    }
    return ret;
}

void MeshSyncClient3dsMax::extractTransformAnimation(ms::Animation& dst_, INode *src)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    ExtractTransform(src, m_current_time, pos, rot, scale);

    float t = TicksToSec(m_current_time) * m_settings.animation_time_scale;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
}

void MeshSyncClient3dsMax::extractCameraAnimation(ms::Animation& dst_, INode *src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = (ms::CameraAnimation&)dst_;
}

void MeshSyncClient3dsMax::extractLightAnimation(ms::Animation& dst_, INode *src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = (ms::LightAnimation&)dst_;
}

void MeshSyncClient3dsMax::extractMeshAnimation(ms::Animation& dst_, INode *src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = (ms::MeshAnimation&)dst_;
}

void MeshSyncClient3dsMax::NodeRecord::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
}

void MeshSyncClient3dsMax::AnimationRecord::operator()(MeshSyncClient3dsMax * _this)
{
    (_this->*extractor)(*dst, src);
}
