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


ms::Identifier MeshSyncClient3dsMax::TreeNode::getIdentifier() const
{
    return { path, id };
}

void MeshSyncClient3dsMax::TreeNode::clearDirty()
{
    dirty_trans = dirty_geom = false;
}

void MeshSyncClient3dsMax::TreeNode::clearState()
{
    dst = nullptr;
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
    m_sender.wait();
    unregisterMenu();

    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
    m_animations.clear();
    m_node_records.clear();
}

void MeshSyncClient3dsMax::onNewScene()
{
    for (auto& kvp : m_node_records) {
        m_entity_manager.erase(kvp.second.getIdentifier());
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
    if (m_settings.auto_sync)
        m_pending_request = SendScope::All;
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
        m_entity_manager.erase(it->second.getIdentifier());
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
            m_entity_manager.erase(kvp.second.getIdentifier());
        }
    }
}

void MeshSyncClient3dsMax::onNodeLinkChanged(INode * n)
{
    m_scene_updated = true;

    auto it = m_node_records.find(n);
    if (it != m_node_records.end()) {
        m_entity_manager.erase(it->second.getIdentifier());
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
        if (sendScene(m_pending_request, false)) {
            m_pending_request = SendScope::None;
        }
    }
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope, bool force_all)
{
    if (m_sender.isSending())
        return false;

    if (force_all) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    if (m_settings.sync_meshes)
        exportMaterials();

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
    m_sender.wait();

    // create default clip
    m_animations.clear();
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
    auto interval = ToTicks(1.0f / std::max(m_settings.animation_sps, 0.01f));
    for (TimeValue t = time_range.Start();;) {
        m_current_time_tick = t;
        m_current_time_sec = ToSeconds(t);
        for (auto& kvp : m_anim_records)
            kvp.second(this);

        if (t >= time_range.End())
            break;
        else
            t = std::min(t + interval, time_range.End());
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

void MeshSyncClient3dsMax::kickAsyncSend()
{
    for (auto& t : m_async_tasks)
        t.wait();
    m_async_tasks.clear();

    for (auto *t : m_tmp_meshes)
        t->DeleteThis();
    m_tmp_meshes.clear();

    for (auto& kvp : m_node_records)
        kvp.second.clearState();

    float to_meter = (float)GetMasterScale(UNITS_METERS);

    // begin async send
    m_sender.on_prepare = [this, to_meter]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::LeftZUp;
        t.scene_settings.scale_factor = m_settings.scale_factor / to_meter;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_sender.on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    m_sender.kick();
}

int MeshSyncClient3dsMax::exportTexture(const std::string & path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

void MeshSyncClient3dsMax::exportMaterials()
{
    auto *mtllib = GetCOREInterface()->GetSceneMtls();
    int count = mtllib->Count();

    int material_index = 0;
    for (int mi = 0; mi < count; ++mi) {
        auto do_export = [this, &material_index](Mtl *mtl) -> int // return material id
        {
            auto dst = ms::Material::create();
            dst->id = m_material_ids.getID(mtl);
            dst->index = material_index++;
            dst->name = mu::ToMBS(mtl->GetName().data());

            auto& dstmat = ms::AsStandardMaterial(*dst);
            dstmat.setColor(to_color(mtl->GetDiffuse()));

            // export textures
            if (m_settings.sync_textures && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
                auto stdmat = (StdMat*)mtl;
                auto export_texture = [this, stdmat](int tid, ms::TextureType ttype) -> int {
                    if (stdmat->MapEnabled(tid)) {
                        auto tex = stdmat->GetSubTexmap(tid);
                        if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
                            MaxSDK::Util::Path path(((BitmapTex*)tex)->GetMapName());
                            path.ConvertToAbsolute();
                            if (path.Exists()) {
                                return exportTexture(mu::ToMBS(path.GetCStr()), ttype);
                            }
                        }
                    }
                    return -1;
                };
                const int DIFFUSE_MAP_ID = 1;
                const int NORMAL_MAP_ID = 8;

                dstmat.setColorMap(export_texture(DIFFUSE_MAP_ID, ms::TextureType::Default));
                dstmat.setBumpMap(export_texture(NORMAL_MAP_ID, ms::TextureType::NormalMap));
            }
            m_material_manager.add(dst);

            return dst->id;
        };

        auto mtlbase = (*mtllib)[mi];
        if (mtlbase->SuperClassID() != MATERIAL_CLASS_ID)
            continue;

        auto mtl = (Mtl*)mtlbase;
        auto& rec = m_material_records[mtl];

        int num_submtls = mtl->NumSubMtls();
        if (num_submtls == 0) {
            rec.material_id = do_export(mtl);
        }
        else {
            for (int si = 0; si < num_submtls; ++si) {
                auto submtl = mtl->GetSubMtl(si);
                if (!submtl)
                    continue;

                auto it = m_material_records.find(submtl);
                if (it != m_material_records.end())
                    rec.submaterial_ids.push_back(it->second.material_id);
                else
                    rec.submaterial_ids.push_back(do_export(submtl));
            }
        }
    }
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}

ms::TransformPtr MeshSyncClient3dsMax::exportObject(INode *n, bool force)
{
    if (!n || !n->GetObjectRef())
        return nullptr;

    auto& rec = getNodeRecord(n);
    if (rec.dst)
        return rec.dst;

    ms::TransformPtr ret;
    // check if the node is instance
    EnumerateInstance(n, [this, &rec, &ret](INode *instance) {
        auto& irec = getNodeRecord(instance);
        if (irec.dst && irec.dst->reference.empty()) {
            ret = exportInstance(rec, irec.dst);
        }
    });
    if (ret)
        return ret;

    auto *obj = GetBaseObject(n);
    rec.baseobj = obj;

    if (IsMesh(obj)) {
        exportObject(n->GetParentNode(), true);

        // export bones
        // this must be before extractMeshData() because meshes can be bones in 3ds Max
        if (m_settings.sync_bones) {
            auto mod = FindSkin(n);
            if (mod && mod->IsEnabled()) {
                auto skin = (ISkin*)mod->GetInterface(I_SKIN);
                EachBone(skin, [this](INode *bone) {
                    exportObject(bone, true);
                });
            }
        }

        if ((m_settings.sync_meshes || m_settings.sync_blendshapes) && rec.dirty_geom) {
            ret = exportMesh(rec);
        }
        else {
            ret = exportTransform(rec);
        }
    }
    else {
        switch (obj->SuperClassID()) {
        case CAMERA_CLASS_ID:
        {
            if (m_settings.sync_cameras) {
                exportObject(n->GetParentNode(), true);
                ret = exportCamera(rec);
            }
            break;
        }
        case LIGHT_CLASS_ID:
        {
            if (m_settings.sync_lights) {
                exportObject(n->GetParentNode(), true);
                ret = exportLight(rec);
            }
            break;
        }
        default:
        {
            if (force) {
                exportObject(n->GetParentNode(), true);
                ret = exportTransform(rec);
            }
            break;
        }
        }
    }

    rec.clearDirty();
    return ret;
}

static mu::float4x4 GetPivotMatrix(INode *n)
{
    auto t = to_float3(n->GetObjOffsetPos());
    auto r = to_quatf(n->GetObjOffsetRot());
    return mu::transform(t, r, mu::float3::one());
}

static void ExtractTransform(INode * n, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    auto mat = to_float4x4(n->GetNodeTM(t));

    // handle parents
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


template<class T>
std::shared_ptr<T> MeshSyncClient3dsMax::createEntity(TreeNode& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = n.path;
    dst.index = n.index;
    n.dst = ret;
    return ret;
}

ms::TransformPtr MeshSyncClient3dsMax::exportTransform(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;

    ExtractTransform(n.node, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr MeshSyncClient3dsMax::exportInstance(TreeNode& n, ms::TransformPtr base)
{
    if (!base)
        return nullptr;

    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;

    ExtractTransform(n.node, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    dst.reference = base->path;
    m_entity_manager.add(ret);
    return ret;
}

ms::CameraPtr MeshSyncClient3dsMax::exportCamera(TreeNode& n)
{
    auto ret = createEntity<ms::Camera>(n);
    auto& dst = *ret;
    ExtractTransform(n.node, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    dst.rotation *= mu::rotateX(-90.0f * mu::Deg2Rad);

    ExtractCameraData((GenCamera*)n.baseobj, GetTime(),
        dst.is_ortho, dst.fov, dst.near_plane, dst.far_plane);
    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr MeshSyncClient3dsMax::exportLight(TreeNode& n)
{
    auto ret = createEntity<ms::Light>(n);
    auto& dst = *ret;
    ExtractTransform(n.node, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    dst.rotation *= mu::rotateX(-90.0f * mu::Deg2Rad);

    ExtractLightData((GenLight*)n.baseobj, GetTime(),
        dst.light_type, dst.color, dst.intensity, dst.spot_angle);
    m_entity_manager.add(ret);
    return ret;
}


static void ExtractNormals(ms::Mesh& dst, Mesh& mesh)
{
    auto* nspec = (MeshNormalSpec*)mesh.GetInterface(MESH_NORMAL_SPEC_INTERFACE);
    if (nspec && nspec->GetFlag(MESH_NORMAL_NORMALS_BUILT)) {
        // there is nspec. I can simply copy normals from it.
        int num_faces = nspec->GetNumFaces();
        auto *faces = nspec->GetFaceArray();
        auto *normals = nspec->GetNormalArray();
        dst.normals.resize_discard(dst.indices.size());

        int ii = 0;
        for (int fi = 0; fi < num_faces; ++fi) {
            auto *idx = faces[fi].GetNormalIDArray();
            for (int ci = 0; ci < 3; ++ci) {
                dst.normals[ii++] = to_float3(normals[idx[ci]]);
            }
        }
    }
    else {
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
        dst.normals.resize_discard(dst.indices.size());

        int num_faces = mesh.numFaces;
        const auto *faces = mesh.faces;
        for (int fi = 0; fi < num_faces; ++fi) {
            auto& face = faces[fi];
            for (int i = 0; i < 3; ++i) {
                dst.normals[fi * 3 + i] = get_normal(fi, face.v[i]);
            }
        }
    }
}

static void GenSmoothNormals(ms::Mesh& dst, Mesh& mesh)
{
    int num_faces = mesh.numFaces;
    const auto *points = (mu::float3*)mesh.verts;
    const auto *faces = mesh.faces;

    // gen face normals
    RawVector<mu::float3> face_normals;
    face_normals.resize_discard(num_faces);
    for (int fi = 0; fi < num_faces; ++fi) {
        const auto& face = faces[fi];
        auto p0 = points[face.v[0]];
        auto p1 = points[face.v[1]];
        auto p2 = points[face.v[2]];
        auto n = mu::cross(p1 - p0, p2 - p0);
        face_normals[fi] = n; // note: not normalized at this point
    }

    // build vertex -> faces connection info
    mu::MeshConnectionInfo connection;
    connection.buildConnection(dst.indices, 3, dst.points);

    dst.normals.resize_discard(dst.indices.size());
    int ii = 0;
    for (int fi = 0; fi < num_faces; ++fi) {
        const auto& face = faces[fi];
        if (face.smGroup != 0) {
            // average normals with neighbor faces that have same smoothing group
            for (int ci = 0; ci < 3; ++ci) {
                auto n = face_normals[fi];
                connection.eachConnectedFaces(face.v[ci], [&](int fi2, int) {
                    if (fi2 == fi)
                        return;
                    const auto& face2 = faces[fi2];
                    if (face.smGroup & face2.smGroup) {
                        n += face_normals[fi2];
                    }
                });
                dst.normals[ii++] = mu::normalize(n);
            }
        }
        else {
            for (int ci = 0; ci < 3; ++ci) {
                auto n = face_normals[fi];
                dst.normals[ii++] = mu::normalize(n);
            }
        }
    }
}

ms::MeshPtr MeshSyncClient3dsMax::exportMesh(TreeNode& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    auto inode = n.node;
    auto& dst = *ret;
    ExtractTransform(inode, GetTime(), dst.position, dst.rotation, dst.scale, dst.visible);
    if (!dst.visible)
        return ret;

    Mesh *mesh = nullptr;
    TriObject *tri = nullptr;
    bool needs_delete = false;

    if (m_settings.sync_meshes) {
        tri = m_settings.bake_modifiers ?
            GetFinalMesh(inode, needs_delete) :
            GetSourceMesh(inode, needs_delete);
        if (tri) {
            mesh = &tri->GetMesh();
            mesh->checkNormals(TRUE);
            if (needs_delete)
                m_tmp_meshes.push_back(tri);
        }
    }

    auto task = [this, ret, inode, mesh]() {
        doExtractMeshData(*ret, inode, mesh);
        m_entity_manager.add(ret);
    };

    if (m_settings.multithreaded)
        m_async_tasks.push_back(std::async(std::launch::async, task));
    else
        task();

    return ret;
}

void MeshSyncClient3dsMax::doExtractMeshData(ms::Mesh & dst, INode *n, Mesh *mesh)
{
    if (mesh) {
        // handle pivot
        dst.refine_settings.flags.apply_local2world = 1;
        dst.refine_settings.local2world = GetPivotMatrix(n);

        // faces
        int num_faces = mesh->numFaces;
        int num_indices = num_faces * 3; // all faces in Mesh are triangle
        {
            dst.counts.clear();
            dst.counts.resize(num_faces, 3);
            dst.material_ids.resize_discard(num_faces);

            const auto& mrec = m_material_records[n->GetMtl()];

            auto *faces = mesh->faces;
            dst.indices.resize_discard(num_indices);
            for (int fi = 0; fi < num_faces; ++fi) {
                auto& face = faces[fi];
                if (!mrec.submaterial_ids.empty()) { // multi-materials
                    int midx = std::min((int)mesh->getFaceMtlIndex(fi), (int)mrec.submaterial_ids.size() - 1);
                    dst.material_ids[fi] = mrec.submaterial_ids[midx];
                }
                else { // single material
                    dst.material_ids[fi] = mrec.material_id;
                }

                for (int i = 0; i < 3; ++i)
                    dst.indices[fi * 3 + i] = face.v[i];
            }
        }

        // points
        int num_vertices = mesh->numVerts;
        dst.points.resize_discard(num_vertices);
        dst.points.assign((mu::float3*)mesh->verts, (mu::float3*)mesh->verts + num_vertices);

        // normals
        if (m_settings.sync_normals) {
            ExtractNormals(dst, *mesh);
        }

        // uv
        if (m_settings.sync_uvs) {
            int num_uv = mesh->numTVerts;
            auto *uv_faces = mesh->tvFace;
            auto *uv_vertices = mesh->tVerts;
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
            int num_colors = mesh->numCVerts;
            auto *vc_faces = mesh->vcFace;
            auto *vc_vertices = mesh->vertCol;
            if (num_colors && vc_faces && vc_vertices) {
                dst.colors.resize_discard(num_indices);
                for (int fi = 0; fi < num_faces; ++fi) {
                    for (int i = 0; i < 3; ++i) {
                        dst.colors[fi * 3 + i] = to_color(vc_vertices[vc_faces[fi].t[i]]);
                    }
                }
            }
        }

        if (!m_settings.bake_modifiers && m_settings.sync_bones) {
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
                    // allocate bones and extract bindposes.
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
    }

    if (!m_settings.bake_modifiers && m_settings.sync_blendshapes) {
        // handle blendshape
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

    {
        dst.setupFlags();
        // request flip faces
        dst.flags.has_refine_settings = 1;
        dst.refine_settings.flags.swap_faces = 1;
        dst.refine_settings.flags.gen_tangents = 1;
        dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
    }
}


ms::AnimationPtr MeshSyncClient3dsMax::exportAnimations(INode *n, bool force)
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
        ret = ms::MeshAnimation::create();
        extractor = &MeshSyncClient3dsMax::extractMeshAnimation;
    }
    else {
        switch (obj->SuperClassID()) {
        case CAMERA_CLASS_ID:
        {
            exportAnimations(n->GetParentNode(), true);
            ret = ms::CameraAnimation::create();
            extractor = &MeshSyncClient3dsMax::extractCameraAnimation;
            break;
        }
        case LIGHT_CLASS_ID:
        {
            exportAnimations(n->GetParentNode(), true);
            ret = ms::LightAnimation::create();
            extractor = &MeshSyncClient3dsMax::extractLightAnimation;
            break;
        }
        default:
        {
            if (force) {
                exportAnimations(n->GetParentNode(), true);
                ret = ms::TransformAnimation::create();
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
        rec.dst = ret;
        rec.node = n;
        rec.obj = obj;
        rec.extractor = extractor;
    }
    return ret;
}

void MeshSyncClient3dsMax::extractTransformAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    bool vis;
    ExtractTransform(src, m_current_time_tick, pos, rot, scale, vis);

    float t = m_current_time_sec * m_settings.animation_time_scale;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
}

void MeshSyncClient3dsMax::extractCameraAnimation(ms::Animation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_current_time_sec * m_settings.animation_time_scale;
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

    float t = m_current_time_sec * m_settings.animation_time_scale;
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

    float t = m_current_time_sec * m_settings.animation_time_scale;
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
