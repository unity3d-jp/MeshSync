#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxUtils.h"
#include "msmaxCallbacks.h"

#ifdef _WIN32
#pragma comment(lib, "core.lib")
#pragma comment(lib, "geom.lib")
#pragma comment(lib, "mesh.lib")
#pragma comment(lib, "poly.lib")
#pragma comment(lib, "mnmath.lib")
#pragma comment(lib, "maxutil.lib")
#pragma comment(lib, "maxscrpt.lib")
#pragma comment(lib, "paramblk2.lib")
#pragma comment(lib, "menus.lib")
#pragma comment(lib, "Morpher.lib")
#endif


static void OnStartup(void *param, NotifyInfo *info)
{
    msmaxInstance().onStartup();
}
static void OnShutdown(void *param, NotifyInfo *info)
{
    msmaxInstance().onShutdown();
}
static void OnNodeRenamed(void *param, NotifyInfo *info)
{
    msmaxInstance().onNodeRenamed();
    msmaxInstance().update();
}
static void OnPreNewScene(void *param, NotifyInfo *info)
{
    msmaxInstance().onNewScene();
}
static void OnPostNewScene(void *param, NotifyInfo *info)
{
    msmaxInstance().update();
}


void MeshSyncClient3dsMax::TreeNode::clearDirty()
{
    dirty_trans = dirty_geom = false;
}

void MeshSyncClient3dsMax::TreeNode::clearState()
{
    dst_obj = nullptr;
}

void MeshSyncClient3dsMax::AnimationRecord::operator()(MeshSyncClient3dsMax * _this)
{
    (_this->*extractor)(*dst, node, obj);
}


MeshSyncClient3dsMax & MeshSyncClient3dsMax::getInstance()
{
    static MeshSyncClient3dsMax s_plugin;
    return s_plugin;
}

MeshSyncClient3dsMax::MeshSyncClient3dsMax()
{
    RegisterNotification(OnStartup, this, NOTIFY_SYSTEM_STARTUP);
    RegisterNotification(OnShutdown, this, NOTIFY_SYSTEM_SHUTDOWN);
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
    // releasing resources is done by onShutdown()
}

MeshSyncClient3dsMax::Settings & MeshSyncClient3dsMax::getSettings()
{
    return m_settings;
}

void MeshSyncClient3dsMax::onStartup()
{
    GetCOREInterface()->RegisterViewportDisplayCallback(TRUE, &msmaxViewportDisplayCallback::getInstance());
    GetCOREInterface()->RegisterTimeChangeCallback(&msmaxTimeChangeCallback::getInstance());
    RegisterNotification(&OnNodeRenamed, this, NOTIFY_NODE_RENAMED);
    RegisterNotification(&OnPreNewScene,  this, NOTIFY_SYSTEM_PRE_RESET );
    RegisterNotification(&OnPostNewScene, this, NOTIFY_SYSTEM_POST_RESET);
    RegisterNotification(&OnPreNewScene,  this, NOTIFY_SYSTEM_PRE_NEW );
    RegisterNotification(&OnPostNewScene, this, NOTIFY_SYSTEM_POST_NEW);
    RegisterNotification(&OnPreNewScene,  this, NOTIFY_FILE_PRE_OPEN );
    RegisterNotification(&OnPostNewScene, this, NOTIFY_FILE_POST_OPEN);
    m_cbkey = GetISceneEventManager()->RegisterCallback(msmaxNodeCallback::getInstance().GetINodeEventCallback());
    registerMenu();
}

void MeshSyncClient3dsMax::onShutdown()
{
    waitAsyncSend();
    unregisterMenu();

    m_objects.clear();
    m_meshes.clear();
    m_materials.clear();
    m_animations.clear();
    m_constraints.clear();
    m_node_records.clear();
}

void MeshSyncClient3dsMax::onNewScene()
{
    for (auto& kvp : m_node_records) {
        m_deleted.push_back(kvp.second.path);
    }
    m_node_records.clear();
    m_scene_updated = true;
}

void MeshSyncClient3dsMax::onSceneUpdated()
{
    m_scene_updated = true;
}

void MeshSyncClient3dsMax::onTimeChanged()
{
    //m_scene_updated = true;
}

void MeshSyncClient3dsMax::onNodeAdded(INode * n)
{
    m_scene_updated = true;
}

void MeshSyncClient3dsMax::onNodeDeleted(INode * n)
{
    m_scene_updated = true;

    auto it = m_node_records.find(n);
    if (it != m_node_records.end()) {
        m_deleted.push_back(it->second.path);
        m_node_records.erase(it);
    }
}

void MeshSyncClient3dsMax::onNodeRenamed()
{
    m_scene_updated = true;

    // search renamed node
    // (event callback tells name of before & after rename, but doesn't tell node itself...)
    for (auto& kvp : m_node_records) {
        if (kvp.second.name != kvp.first->GetName()) {
            m_deleted.push_back(kvp.second.path);
        }
    }
}

void MeshSyncClient3dsMax::onNodeLinkChanged(INode * n)
{
    m_scene_updated = true;

    auto it = m_node_records.find(n);
    if (it != m_node_records.end()) {
        m_deleted.push_back(it->second.path);
    }
}

void MeshSyncClient3dsMax::onNodeUpdated(INode * n)
{
    auto& rec = getNodeRecord(n);
    m_dirty = rec.dirty_trans = true;
}

void MeshSyncClient3dsMax::onGeometryUpdated(INode * n)
{
    auto& rec = getNodeRecord(n);
    m_dirty = rec.dirty_trans = rec.dirty_geom = true;
}

void MeshSyncClient3dsMax::onRepaint()
{
    update();
}


void MeshSyncClient3dsMax::update()
{
    if (m_scene_updated) {
        updateRecords();
        m_scene_updated = false;
        if (m_settings.auto_sync) {
            m_pending_request = SendScope::All;
        }
    }
    if (m_settings.auto_sync && m_pending_request == SendScope::None && m_dirty) {
        m_pending_request = SendScope::Updated;
    }

    if (m_pending_request != SendScope::None) {
        if (sendScene(m_pending_request)) {
            m_pending_request = SendScope::None;
        }
    }
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope)
{
    if (isSending()) {
        return false;
    }

    int num_exported = 0;
    if (scope == SendScope::All) {
        for (auto& kvp : m_node_records) {
            kvp.second.dirty_trans = kvp.second.dirty_geom = true;
            if (exportObject(kvp.first, false))
                ++num_exported;
        }
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_node_records) {
            auto& rec = kvp.second;
            if (rec.dirty_trans || rec.dirty_geom) {
                if (exportObject(kvp.first, true))
                    ++num_exported;
            }
        }
    }

    if (num_exported > 0)
        kickAsyncSend();

    // cleanup intermediate data
    m_material_records.clear();

    m_dirty = false;
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
            if (exportAnimations(n, false))
                ++num_exported;
        });
    }
    else {
        // todo:
    }

    // advance frame and record animation
    auto time_range = GetCOREInterface()->GetAnimRange();
    auto interval = ToTicks(1.0f / m_settings.animation_sps);
    for (TimeValue t = time_range.Start(); t <= time_range.End(); t += interval) {
        m_current_time_tick = t;
        m_current_time_sec = ToSeconds(t);
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

    if (!m_animations.empty())
        kickAsyncSend();
    return true;
}

bool MeshSyncClient3dsMax::recvScene()
{
    return false;
}


void MeshSyncClient3dsMax::updateRecords()
{
    m_node_records.clear();
    EnumerateAllNode([this](INode *n) {
        getNodeRecord(n);
    });
}

MeshSyncClient3dsMax::TreeNode & MeshSyncClient3dsMax::getNodeRecord(INode *n)
{
    auto& rec = m_node_records[n];
    if (rec.index == 0) {
        rec.index = ++m_index_seed;
        rec.node = n;
        rec.name = GetNameW(n);
        rec.path = GetPath(n);
    }
    return rec;
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

    float to_meter = (float)GetMasterScale(UNITS_METERS);

    // begin async send
    m_future_send = std::async(std::launch::async, [this, to_meter]() {
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::LeftZUp;
        scene_settings.scale_factor = m_settings.scale_factor / to_meter;

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

void MeshSyncClient3dsMax::exportMaterials()
{
    auto *mtllib = GetCOREInterface()->GetSceneMtls();
    int count = mtllib->Count();

    m_materials.clear();
    for (int mi = 0; mi < count; ++mi) {
        auto do_export = [this](Mtl *mtl) -> int // return material id
        {
            int mid = (int)m_materials.size();
            auto dst = ms::Material::create();
            m_materials.push_back(dst);
            dst->name = mu::ToMBS(mtl->GetName().data());
            dst->color = to_color(mtl->GetDiffuse());
            return mid;
        };

        auto mtl = (Mtl*)(*mtllib)[mi];
        auto& rec = m_material_records[mtl];

        int num_submtls = mtl->NumSubMtls();
        if (num_submtls == 0) {
            rec.material_id = do_export(mtl);
        }
        else {
            for (int si = 0; si < num_submtls; ++si) {
                auto submtl = mtl->GetSubMtl(si);
                auto it = m_material_records.find(submtl);
                if (it != m_material_records.end())
                    rec.submaterial_ids.push_back(it->second.material_id);
                else
                    rec.submaterial_ids.push_back(do_export(submtl));
            }
        }
    }
}


ms::Transform* MeshSyncClient3dsMax::exportObject(INode * n, bool force)
{
    if (!n || !n->GetObjectRef())
        return nullptr;

    auto *obj = GetBaseObject(n);
    auto& rec = getNodeRecord(n);
    if (rec.dst_obj)
        return rec.dst_obj;

    ms::TransformPtr ret;
    if (IsMesh(obj) && (m_settings.sync_meshes || m_settings.sync_bones)) {
        exportObject(n->GetParentNode(), true);

        if (m_settings.sync_meshes && rec.dirty_geom) {
            auto dst = ms::Mesh::create();
            ret = dst;
            m_meshes.push_back(dst);
            extractMeshData(*dst, n, obj);
        }
        else {
            auto dst = ms::Transform::create();
            ret = dst;
            m_objects.push_back(dst);
            extractTransformData(*dst, n, obj);
        }

        if (m_settings.sync_bones) {
            auto mod = FindSkin(n);
            if (mod && mod->IsEnabled()) {
                auto skin = (ISkin*)mod->GetInterface(I_SKIN);
                // export bone nodes
                EachBone(skin, [this](INode *bone) {
                    exportObject(bone, true);
                });
            }
        }
    }
    else {
        switch (obj->SuperClassID()) {
        case CAMERA_CLASS_ID:
        {
            if (m_settings.sync_cameras) {
                exportObject(n->GetParentNode(), true);
                auto dst = ms::Camera::create();
                ret = dst;
                m_objects.push_back(dst);
                extractCameraData(*dst, n, obj);
            }
            break;
        }
        case LIGHT_CLASS_ID:
        {
            if (m_settings.sync_lights) {
                exportObject(n->GetParentNode(), true);
                auto dst = ms::Light::create();
                ret = dst;
                m_objects.push_back(dst);
                extractLightData(*dst, n, obj);
            }
            break;
        }
        default:
        {
            if (force) {
                exportObject(n->GetParentNode(), true);
                auto dst = ms::Transform::create();
                ret = dst;
                m_objects.push_back(dst);
                extractTransformData(*dst, n, obj);
            }
            break;
        }
        }
    }

    if (ret) {
        ret->path = rec.path;
        ret->index = rec.index;
        rec.dst_obj = ret.get();
    }
    rec.clearDirty();
    return ret.get();
}


static void ExtractTransform(INode * n, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    auto mat = to_float4x4(n->GetNodeTM(t));
    if (auto parent = n->GetParentNode()) {
        auto pmat = to_float4x4(parent->GetNodeTM(t));
        mat *= mu::invert(pmat);
    }
    pos = mu::extract_position(mat);
    rot = mu::extract_rotation(mat);
    scale = mu::extract_scale(mat);
    vis = !n->IsHidden();
}

static void ExtractCameraData(GenCamera *cam, TimeValue t,
    bool& ortho, float& fov, float& near_plane, float& far_plane)
{
    ortho = cam->IsOrtho();
    {
        float hfov = cam->GetFOV(t);
        // CameraObject::GetFOV() returns horizontal fov. we need vertical one.
        float vfov = 2.0f * std::atan(std::tan(hfov / 2.0f) / (GetCOREInterface()->GetRendImageAspect()));
        fov = vfov * mu::Rad2Deg;
    }
    if (cam->GetManualClip()) {
        near_plane = cam->GetClipDist(t, 0);
        far_plane = cam->GetClipDist(t, 1);
    }
    else {
        near_plane = far_plane = 0.0f;
    }
}

static void ExtractLightData(GenLight *light, TimeValue t,
    ms::Light::LightType& type, mu::float4& color, float& intensity, float& spot_angle)
{
    switch (light->Type()) {
    case TSPOT_LIGHT:
    case FSPOT_LIGHT: // fall through
    {
        type = ms::Light::LightType::Spot;
        spot_angle = light->GetHotspot(t);
        break;
    }
    case DIR_LIGHT:
    case TDIR_LIGHT: // fall through
    {
        type = ms::Light::LightType::Directional;
        break;
    }
    case OMNI_LIGHT:
    default: // fall through
    {
        type = ms::Light::LightType::Point;
        break;
    }
    }

    (mu::float3&)color = to_float3(light->GetRGBColor(t));
    intensity = light->GetIntensity(t);
}


bool MeshSyncClient3dsMax::extractTransformData(ms::Transform &dst, INode *n, Object * /*obj*/)
{
    ExtractTransform(n, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    return true;
}

bool MeshSyncClient3dsMax::extractCameraData(ms::Camera &dst, INode *n, Object *obj)
{
    extractTransformData(dst, n, obj);
    dst.rotation *= mu::rotateX(-90.0f * mu::Deg2Rad);

    ExtractCameraData((GenCamera*)obj, GetTime(),
        dst.is_ortho, dst.fov, dst.near_plane, dst.far_plane);
    return true;
}

bool MeshSyncClient3dsMax::extractLightData(ms::Light &dst, INode *n, Object *obj)
{
    extractTransformData(dst, n, obj);
    dst.rotation *= mu::rotateX(-90.0f * mu::Deg2Rad);

    ExtractLightData((GenLight*)obj, GetTime(),
        dst.light_type, dst.color, dst.intensity, dst.spot_angle);
    return true;
}


// dst must be allocated with length of indices
static void ExtractNormals(RawVector<mu::float3> &dst, Mesh & mesh)
{
    auto* nspec = (MeshNormalSpec*)mesh.GetInterface(MESH_NORMAL_SPEC_INTERFACE);
    if (nspec && nspec->GetFlag(MESH_NORMAL_NORMALS_BUILT)) {
        int num_faces = nspec->GetNumFaces();
        auto *faces = nspec->GetFaceArray();
        auto *normals = nspec->GetNormalArray();
        int ii = 0;
        for (int fi = 0; fi < num_faces; ++fi) {
            auto *idx = faces[fi].GetNormalIDArray();
            for (int ci = 0; ci < 3; ++ci) {
                dst[ii++] = to_float3(normals[idx[ci]]);
            }
        }
    }
    else {
        // copied from SDK samples...
        auto get_normal = [&mesh](int face_index, int vertex_index) -> mu::float3 {
            const auto& rv = mesh.getRVert(vertex_index);
            const auto& face = mesh.faces[face_index];
            DWORD smGroup = face.smGroup;
            int num_normals = 0;
            Point3 ret;

            if (rv.rFlags & SPECIFIED_NORMAL) {
                ret = rv.rn.getNormal();
            }
            else if ((num_normals = rv.rFlags & NORCT_MASK) != 0 && smGroup) {
                if (num_normals == 1) {
                    ret = rv.rn.getNormal();
                }
                else {
                    for (int i = 0; i < num_normals; i++) {
                        if (rv.ern[i].getSmGroup() & smGroup) {
                            ret = rv.ern[i].getNormal();
                        }
                    }
                }
            }
            else {
                ret = mesh.getFaceNormal(face_index);
            }
            return to_float3(ret);
        };

        // make sure normal is allocated
        mesh.checkNormals(TRUE);

        int num_faces = mesh.numFaces;
        const auto *faces = mesh.faces;
        for (int fi = 0; fi < num_faces; ++fi) {
            auto& face = faces[fi];
            for (int i = 0; i < 3; ++i) {
                dst[fi * 3 + i] = get_normal(fi, face.v[i]);
            }
        }
    }
}

bool MeshSyncClient3dsMax::extractMeshData(ms::Mesh & dst, INode * n, Object *obj)
{
    extractTransformData(dst, n, obj);
    if (!dst.visible)
        return true;

    auto *tri = GetSourceMesh(n);
    if (!tri)
        return false;

    if (m_materials.empty())
        exportMaterials();

    auto& mesh = tri->GetMesh();
    doExtractMeshData(dst, n, mesh);

    return true;
}

void MeshSyncClient3dsMax::doExtractMeshData(ms::Mesh & dst, INode * n, Mesh & mesh)
{
    // faces
    int num_faces = mesh.numFaces;
    int num_indices = num_faces * 3; // all faces in Mesh are triangle
    {
        dst.counts.clear();
        dst.counts.resize(num_faces, 3);
        dst.material_ids.resize_discard(num_faces);

        const auto& mrec = m_material_records[n->GetMtl()];

        auto *faces = mesh.faces;
        dst.indices.resize_discard(num_indices);
        for (int fi = 0; fi < num_faces; ++fi) {
            auto& face = faces[fi];
            if (!mrec.submaterial_ids.empty()) { // multi-materials
                int mid = std::min((int)mesh.getFaceMtlIndex(fi), (int)mrec.submaterial_ids.size() - 1);
                dst.material_ids[fi] = mrec.submaterial_ids[mid];
            }
            else { // single material
                dst.material_ids[fi] = mrec.material_id;
            }

            for (int i = 0; i < 3; ++i)
                dst.indices[fi * 3 + i] = face.v[i];
        }
    }

    // points
    int num_vertices = mesh.numVerts;
    dst.points.resize_discard(num_vertices);
    dst.points.assign((mu::float3*)mesh.verts, (mu::float3*)mesh.verts + num_vertices);

    // normals
    if (m_settings.sync_normals) {
        dst.normals.resize_discard(num_indices);
        ExtractNormals(dst.normals, mesh);
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

    // handle blendshape
    if (m_settings.sync_blendshapes) {
        auto *mod = FindMorph(n);
        if (mod && mod->IsEnabled()) {
            int num_faces = (int)dst.counts.size();
            int num_points = (int)dst.points.size();
            int num_normals = (int)dst.normals.size();

            MaxMorphModifier morph(mod);
            int num_channels = morph.NumMorphChannels();
            for (int ci = 0; ci < num_channels; ++ci) {
                auto channel = morph.GetMorphChannel(ci);
                auto num_targets = channel.NumProgressiveMorphTargets();
                if (!channel.IsActive() || !channel.IsValid() || num_targets == 0 || channel.NumMorphPoints() != num_points)
                    continue;

                auto dbs = ms::BlendShapeData::create();
                for (int ti = 0; ti < num_targets; ++ti) {
                    dbs->frames.push_back(ms::BlendShapeFrameData::create());
                    auto& frame = *dbs->frames.back();
                    frame.weight = channel.GetProgressiveMorphWeight(ti);

                    // gen delta
                    frame.points.resize_discard(num_points);
                    for (int vi = 0; vi < num_points; ++vi)
                        frame.points[vi] = to_float3(channel.GetProgressiveMorphPoint(ti, vi)) - dst.points[vi];
                }
                if (!dbs->frames.empty()) {
                    dbs->name = mu::ToMBS(channel.GetName());
                    dbs->weight = channel.GetMorphWeight(GetTime());
                    dst.blendshapes.push_back(dbs);
                }
            }
        }
    }

    if (m_settings.sync_bones) {
        auto *mod = FindSkin(n);
        if (mod && mod->IsEnabled()) {
            auto skin = (ISkin*)mod->GetInterface(I_SKIN);
            auto ctx = skin->GetContextInterface(n);
            int num_bones = skin->GetNumBones();
            int num_vertices = ctx->GetNumPoints();
            if (num_vertices != dst.points.size()) {
                // topology is changed by modifiers. this case is not supported.
            }
            else {
                // allocate bones and extract bindposes
                // note: in max, bindpose is [skin_matrix * inv_bone_matrix]
                Matrix3 skin_matrix;
                skin->GetSkinInitTM(n, skin_matrix);
                for (int bi = 0; bi < num_bones; ++bi) {
                    auto bone = skin->GetBone(bi);
                    Matrix3 bone_matrix;
                    skin->GetBoneInitTM(bone, bone_matrix);

                    auto bd = ms::BoneData::create();
                    dst.bones.push_back(bd);
                    bd->bindpose = to_float4x4(skin_matrix) * mu::invert(to_float4x4(bone_matrix));
                    bd->weights.resize_zeroclear(dst.points.size()); // allocate weights

                    auto bit = m_node_records.find(bone);
                    if (bit != m_node_records.end()) {
                        bd->path = bit->second.path;
                    }
                }

                // get weights
                for (int vi = 0; vi < num_vertices; ++vi) {
                    int num_affected_bones = ctx->GetNumAssignedBones(vi);
                    for (int bi = 0; bi < num_affected_bones; ++bi) {
                        int bone_index = ctx->GetAssignedBone(vi, bi);
                        float bone_weight = ctx->GetBoneWeight(vi, bi);
                        dst.bones[bone_index]->weights[vi] = bone_weight;
                    }
                }
            }
        }
    }

    {
        dst.setupFlags();
        // request flip faces
        dst.flags.has_refine_settings = 1;
        dst.refine_settings.flags.swap_faces = 1;
    }
}

ms::Animation* MeshSyncClient3dsMax::exportAnimations(INode * n, bool force)
{
    if (!n || !n->GetObjectRef())
        return nullptr;

    auto it = m_anim_records.find(n);
    if (it != m_anim_records.end())
        return it->second.dst;

    auto obj = GetBaseObject(n);
    auto& animations = m_animations[0]->animations;
    ms::AnimationPtr ret;
    AnimationRecord::extractor_t extractor = nullptr;

    if (IsMesh(obj)) {
        exportAnimations(n->GetParentNode(), true);
        auto mod = FindSkin(n);
        if (mod && mod->IsEnabled()) {
            auto skin = (ISkin*)mod->GetInterface(I_SKIN);
            EachBone(skin, [this](INode *bone) {
                exportAnimations(bone, true);
            });
        }
        auto dst = ms::MeshAnimation::create();
        ret = dst;
        extractor = &MeshSyncClient3dsMax::extractMeshAnimation;
    }
    else {
        switch (obj->SuperClassID()) {
        case CAMERA_CLASS_ID:
        {
            exportAnimations(n->GetParentNode(), true);
            auto dst = ms::CameraAnimation::create();
            ret = dst;
            extractor = &MeshSyncClient3dsMax::extractCameraAnimation;
            break;
        }
        case LIGHT_CLASS_ID:
        {
            exportAnimations(n->GetParentNode(), true);
            auto dst = ms::LightAnimation::create();
            ret = dst;
            extractor = &MeshSyncClient3dsMax::extractLightAnimation;
            break;
        }
        default:
        {
            if (force) {
                exportAnimations(n->GetParentNode(), true);
                auto dst = ms::TransformAnimation::create();
                ret = dst;
                extractor = &MeshSyncClient3dsMax::extractTransformAnimation;
            }
            break;
        }
        }
    }

    if (ret) {
        animations.push_back(ret);
        ret->path = GetPath(n);

        auto& rec = m_anim_records[n];
        rec.dst = ret.get();
        rec.node = n;
        rec.obj = obj;
        rec.extractor = extractor;
    }
    return ret.get();
}

void MeshSyncClient3dsMax::extractTransformAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    bool vis;
    ExtractTransform(src, m_current_time_tick, pos, rot, scale, vis);

    float t = m_current_time_sec;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
}

void MeshSyncClient3dsMax::extractCameraAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_current_time_sec;
    auto& dst = (ms::CameraAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value *= mu::rotateX(-90.0f * mu::Deg2Rad);
    }

    bool ortho;
    float fov, near_plane, far_plane;
    ExtractCameraData((GenCamera*)obj, m_current_time_tick, ortho, fov, near_plane, far_plane);

    dst.fov.push_back({ t, fov });
    dst.near_plane.push_back({ t, near_plane });
    dst.far_plane.push_back({ t, far_plane });
}

void MeshSyncClient3dsMax::extractLightAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_current_time_sec;
    auto& dst = (ms::LightAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value *= mu::rotateX(-90.0f * mu::Deg2Rad);
    }
    ms::Light::LightType type;
    mu::float4 color;
    float intensity, spot_angle;
    ExtractLightData((GenLight*)obj, m_current_time_tick, type, color, intensity, spot_angle);

    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (type == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void MeshSyncClient3dsMax::extractMeshAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_current_time_sec;
    auto& dst = (ms::MeshAnimation&)dst_;

    if (m_settings.sync_blendshapes) {
        auto *mod = FindMorph(src);
        if (mod) {
            MaxMorphModifier morph(mod);
            int num_channels = morph.NumMorphChannels();
            for (int ci = 0; ci < num_channels; ++ci) {
                auto channel = morph.GetMorphChannel(ci);
                auto tnode = channel.GetMorphTarget();
                if (!tnode || !channel.IsActive() || !channel.IsValid())
                    continue;

                auto name = mu::ToMBS(channel.GetName());
                auto dbs = dst.findOrCreateBlendshapeAnimation(name.c_str());
                dbs->weight.push_back({ t, channel.GetMorphWeight(m_current_time_tick) });
            }
        }
    }
}
