#include "pch.h"
#include "msmaxContext.h"
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
    msmaxGetContext().onStartup();
}
static void OnShutdown(void *param, NotifyInfo *info)
{
    msmaxGetContext().onShutdown();
}
static void OnNodeRenamed(void *param, NotifyInfo *info)
{
    msmaxGetContext().onNodeRenamed();
    msmaxGetContext().update();
}
static void OnPreNewScene(void *param, NotifyInfo *info)
{
    msmaxGetContext().onNewScene();
}
static void OnPostNewScene(void *param, NotifyInfo *info)
{
    msmaxGetContext().update();
}

static void FeedDeferredCallsImpl(void*)
{
    msmaxGetContext().feedDeferredCalls();
}
static void FeedDeferredCalls()
{
    // call FeedDeferredCallsImpl from main thread
    // https://help.autodesk.com/view/3DSMAX/2017/ENU/?guid=__files_GUID_0FA27485_D808_40B7_8465_B1C293077597_htm
    const UINT WM_TRIGGER_CALLBACK = WM_USER + 4764;
    ::PostMessage(GetCOREInterface()->GetMAXHWnd(), WM_TRIGGER_CALLBACK, (WPARAM)&FeedDeferredCallsImpl, (LPARAM)nullptr);
}


ms::Identifier msmaxContext::TreeNode::getIdentifier() const
{
    return { path, id };
}

void msmaxContext::TreeNode::clearDirty()
{
    dirty_trans = dirty_geom = false;
}

void msmaxContext::TreeNode::clearState()
{
    dst = nullptr;
}

void msmaxContext::AnimationRecord::operator()(msmaxContext * _this)
{
    (_this->*extractor)(*dst, node, obj);
}


msmaxContext & msmaxContext::getInstance()
{
    static msmaxContext s_plugin;
    return s_plugin;
}

msmaxContext::msmaxContext()
{
    RegisterNotification(OnStartup, this, NOTIFY_SYSTEM_STARTUP);
    RegisterNotification(OnShutdown, this, NOTIFY_SYSTEM_SHUTDOWN);
}

msmaxContext::~msmaxContext()
{
    // releasing resources is done by onShutdown()
}

msmaxSettings& msmaxContext::getSettings()
{
    return m_settings;
}

void msmaxContext::onStartup()
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

void msmaxContext::onShutdown()
{
    wait();
    unregisterMenu();

    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
    m_animations.clear();
    m_node_records.clear();
}

void msmaxContext::onNewScene()
{
    for (auto& kvp : m_node_records) {
        m_entity_manager.erase(kvp.second.getIdentifier());
    }
    m_node_records.clear();
    m_scene_updated = true;
}

void msmaxContext::onSceneUpdated()
{
    m_scene_updated = true;
}

void msmaxContext::onTimeChanged()
{
    if (m_settings.auto_sync)
        m_pending_request = SendScope::All;
}

void msmaxContext::onNodeAdded(INode * n)
{
    m_scene_updated = true;
}

void msmaxContext::onNodeDeleted(INode * n)
{
    m_scene_updated = true;

    auto it = m_node_records.find(n);
    if (it != m_node_records.end()) {
        m_entity_manager.erase(it->second.getIdentifier());
        m_node_records.erase(it);
    }
}

void msmaxContext::onNodeRenamed()
{
    m_scene_updated = true;
}

void msmaxContext::onNodeLinkChanged(INode *n)
{
    m_scene_updated = true;
}

void msmaxContext::onNodeUpdated(INode *n)
{
    auto& rec = getNodeRecord(n);
    m_dirty = rec.dirty_trans = true;
}

void msmaxContext::onGeometryUpdated(INode *n)
{
    auto& rec = getNodeRecord(n);
    m_dirty = rec.dirty_trans = rec.dirty_geom = true;
}

void msmaxContext::onRepaint()
{
    update();
}

void msmaxContext::logInfo(const char * format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    {
        auto mes = mu::ToWCS(buf);
        the_listener->edit_stream->wputs(mes.c_str());
        the_listener->edit_stream->wflush();
    }
    va_end(args);
}

bool msmaxContext::isServerAvailable()
{
    m_sender.client_settings = m_settings.client_settings;
    return m_sender.isServerAvaileble();
}

const std::string& msmaxContext::getErrorMessage()
{
    return m_sender.getErrorMessage();
}


void msmaxContext::wait()
{
    m_sender.wait();
}

void msmaxContext::update()
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
        if (sendObjects(m_pending_request, false)) {
            m_pending_request = SendScope::None;
        }
    }
}

bool msmaxContext::sendObjects(SendScope scope, bool dirty_all)
{
    if (m_sender.isExporting())
        return false;

    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy

    if (m_settings.sync_meshes)
        exportMaterials();

    int num_exported = 0;
    auto nodes = getNodes(scope);
    for (auto& n : nodes) {
        if (exportObject(n->node, true))
            ++num_exported;
    }

    if (num_exported > 0)
        kickAsyncSend();

    // cleanup intermediate data
    m_material_records.clear();

    m_dirty = false;
    return true;
}

bool msmaxContext::sendMaterials(bool dirty_all)
{
    if (m_sender.isExporting())
        return false;

    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(dirty_all);
    exportMaterials();

    // send
    kickAsyncSend();

    // cleanup intermediate data
    m_material_records.clear();
    return true;
}

bool msmaxContext::sendAnimations(SendScope scope)
{
    m_sender.wait();

    // create default clip
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create());

    auto& clip = *m_animations.back();
    clip.frame_rate = (float)::GetFrameRate();

    // gather target data
    int num_exported = 0;
    auto nodes = getNodes(scope);
    for (auto n : nodes) {
        if (exportAnimations(n->node, false))
            ++num_exported;
    }

    // advance frame and record animation
    auto time_range = GetCOREInterface()->GetAnimRange();
    auto time_start = time_range.Start();
    auto time_end = time_range.End();
    auto interval = ToTicks(1.0f / std::max(m_settings.animation_sps, 0.01f));
    for (TimeValue t = time_start;;) {
        m_current_time_tick = t;
        m_anim_time = ToSeconds(t - time_start) * m_settings.animation_time_scale;
        for (auto& kvp : m_anim_records)
            kvp.second(this);

        if (t >= time_range.End())
            break;
        else
            t = std::min(t + interval, time_end);
    }

    // cleanup intermediate data
    m_anim_records.clear();

    if (m_settings.keyframe_reduction) {
        // keyframe reduction
        for (auto& clip : m_animations)
            clip->reduction(m_settings.keep_flat_curves);

        // erase empty animation
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
            m_animations.end());
    }

    if (!m_animations.empty())
        kickAsyncSend();
    return true;
}


static DWORD WINAPI CB_Dummy(LPVOID arg) { return 0; }

bool msmaxContext::writeCache(SendScope scope, bool all_frames, const std::string& path)
{
    ms::OSceneCacheSettings oscs;
    oscs.sample_rate = m_settings.animation_sps;
    oscs.encoder_settings.zstd.compression_level = 100;
    oscs.flatten_hierarchy = m_settings.sc_flatten_hierarchy;
    //oscs.apply_refinement = 0;
    //oscs.strip_normals = 1;
    oscs.strip_tangents = 1;
    if (!m_cache_writer.open(path.c_str(), oscs))
        return false;

    auto settings_old = m_settings;
    m_settings.export_scene_cache = true;
    m_settings.bake_modifiers = true;
    m_settings.use_render_meshes = true;

    m_material_manager.setAlwaysMarkDirty(true);
    m_entity_manager.setAlwaysMarkDirty(true);
    m_texture_manager.clearDirtyFlags();

    bool export_materials = m_settings.sc_material_scope != msmaxMaterialScope::None;
    auto nodes = getNodes(scope);

    auto do_export = [&]() {
        if (export_materials) {
            exportMaterials();
            if (m_settings.sc_material_scope != msmaxMaterialScope::AllFrames)
                m_material_manager.clearDirtyFlags();
        }

        int num_exported = 0;
        auto export_objects = [&]() {
            for (auto& n : nodes) {
                if (exportObject(n->node, true))
                    ++num_exported;
            }
        };

        if (m_settings.use_render_meshes) {
            for (auto& n : nodes)
                m_render_scope.addNode(n->node);
            m_render_scope.prepare(GetTime());
            m_render_scope.scope(export_objects);
        }
        else {
            export_objects();
        }
        kickAsyncSend();

        // cleanup intermediate data
        m_material_records.clear();
    };

    auto *ifs = GetCOREInterface();
    if (all_frames) {
        ifs->ProgressStart(L"Exporting Scene Cache", TRUE, CB_Dummy, nullptr);

        auto time_range = ifs->GetAnimRange();
        auto time_start = time_range.Start();
        auto time_end = time_range.End();
        auto interval = ToTicks(1.0f / std::max(m_settings.animation_sps, 0.01f));
        for (TimeValue t = time_start;;) {
            m_current_time_tick = t;
            m_anim_time = ToSeconds(t - time_start);
            ifs->SetTime(m_current_time_tick);

            do_export();

            float progress = float(m_current_time_tick - time_start) / float(time_end - time_start) * 100.0f;
            ifs->ProgressUpdate((int)progress);

            if (t >= time_range.End()) {
                break;
            }
            else if (ifs->GetCancel()) {
                ifs->SetCancel(FALSE);
                break;
            }
            else {
                t = std::min(t + interval, time_end);
            }
        }
        ifs->ProgressEnd();
    }
    else {
        m_anim_time = 0.0f;
        do_export();
    }

    m_settings = settings_old;
    m_cache_writer.close();
    return true;
}

bool msmaxContext::recvScene()
{
    return false;
}


void msmaxContext::updateRecords()
{
    struct ExistRecord
    {
        std::string path;
        bool exists;

        bool operator<(const ExistRecord& v) const { return path < v.path; }
        bool operator<(const std::string& v) const { return path < v; }
    };
    // create path list to detect rename / re-parent 
    std::vector<ExistRecord> old_records;
    old_records.reserve(m_node_records.size());
    for (auto& kvp : m_node_records)
        old_records.push_back({ std::move(kvp.second.path), false });
    std::sort(old_records.begin(), old_records.end());

    // re-construct records
    m_node_records.clear();
    EnumerateAllNode([this](INode *n) {
        getNodeRecord(n);
    });

    // erase renamed / re-parented objects
    for (auto& kvp : m_node_records) {
        auto it = std::lower_bound(old_records.begin(), old_records.end(), kvp.second.path);
        if (it != old_records.end() && it->path == kvp.second.path)
            it->exists = true;
    }
    for (auto& r : old_records) {
        if (!r.exists)
            m_entity_manager.erase(r.path);
    }
}

msmaxContext::TreeNode & msmaxContext::getNodeRecord(INode *n)
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

std::vector<msmaxContext::TreeNode*> msmaxContext::getNodes(SendScope scope)
{
    std::vector<TreeNode*> ret;
    ret.reserve(m_node_records.size());

    switch (scope)
    {
    case SendScope::All:
        for (auto& kvp : m_node_records)
            ret.push_back(&kvp.second);
        break;
    case SendScope::Updated:
        for (auto& kvp : m_node_records) {
            if (kvp.second.dirty_trans || kvp.second.dirty_geom)
                ret.push_back(&kvp.second);
        }
        break;
    case SendScope::Selected:
        for (auto& kvp : m_node_records) {
            if (kvp.second.node->Selected())
                ret.push_back(&kvp.second);
        }
        break;
    }
    return ret;
}

void msmaxContext::kickAsyncSend()
{
    for (auto& t : m_async_tasks)
        t.wait();
    m_async_tasks.clear();

    for (auto *t : m_tmp_triobj)
        t->DeleteMe();
    m_tmp_triobj.clear();

    for (auto *t : m_tmp_meshes)
        t->DeleteThis();
    m_tmp_meshes.clear();

    for (auto& kvp : m_node_records)
        kvp.second.clearState();


    float to_meter = (float)GetMasterScale(UNITS_METERS);
    using Exporter = ms::AsyncSceneExporter;
    Exporter *exporter = m_settings.export_scene_cache ? (Exporter*)&m_cache_writer : (Exporter*)&m_sender;

    // begin async send
    exporter->on_prepare = [this, to_meter, exporter]() {
        if (auto sender = dynamic_cast<ms::AsyncSceneSender*>(exporter)) {
            sender->client_settings = m_settings.client_settings;
        }
        else if (auto writer = dynamic_cast<ms::AsyncSceneCacheWriter*>(exporter)) {
            writer->time = m_anim_time;
        }

        auto& t = *exporter;
        t.scene_settings.handedness = ms::Handedness::RightZUp;
        t.scene_settings.scale_factor = m_settings.scale_factor / to_meter;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };

    exporter->on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };

    exporter->kick();
}

int msmaxContext::exportTexture(const std::string & path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

void msmaxContext::exportMaterials()
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

ms::TransformPtr msmaxContext::exportObject(INode *n, bool tip)
{
    if (!n || !n->GetObjectRef())
        return nullptr;

    auto& rec = getNodeRecord(n);
    if (rec.dst)
        return nullptr;

    auto *obj = GetBaseObject(n);
    rec.baseobj = obj;
    ms::TransformPtr ret;

    auto handle_parent = [&]() {
        exportObject(n->GetParentNode(), false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        ret = exportTransform(rec);
    };
    auto handle_instance = [&]() -> bool {
        // always make per-instance meshes if 'bake modifiers' mode
        if (m_settings.bake_modifiers)
            return false;

        // check if the node is instance
        EachInstance(n, [this, &rec, &ret](INode *instance) {
            if (ret || (m_settings.ignore_non_renderable && !IsRenderable(instance)))
                return;
            auto& irec = getNodeRecord(instance);
            if (irec.dst && irec.dst->reference.empty())
                ret = exportInstance(rec, irec.dst);
        });
        return ret != nullptr;
    };


    if (IsMesh(obj) && (!m_settings.ignore_non_renderable || IsRenderable(n))) {
        // export bones
        // this must be before extractMeshData() because meshes can be bones in 3ds Max
        if (m_settings.sync_bones && !m_settings.bake_modifiers) {
            EachBone(n, [this](INode *bone) {
                exportObject(bone, false);
            });
        }

        if (m_settings.sync_meshes || m_settings.sync_blendshapes) {
            handle_parent();
            if (!handle_instance())
                ret = exportMesh(rec);
        }
        else if (!tip)
            handle_transform();
    }
    else {
        if (IsCamera(obj) && m_settings.sync_cameras) {
            handle_parent();
            ret = exportCamera(rec);
        }
        else if (IsLight(obj) && m_settings.sync_lights) {
            handle_parent();
            ret = exportLight(rec);
        }
        else if (!tip) {
            handle_transform();
        }
    }

    rec.clearDirty();
    return ret;
}

mu::float4x4 msmaxContext::getPivotMatrix(INode *n)
{
    auto t = to_float3(n->GetObjOffsetPos());
    auto r = to_quatf(n->GetObjOffsetRot());
    return mu::transform(t, r, mu::float3::one());
}

mu::float4x4 msmaxContext::getGlobalMatrix(INode *n, TimeValue t)
{
    if (m_settings.bake_modifiers) {
        auto m = n->GetObjTMAfterWSM(t);
        if (m.IsIdentity())
            m = n->GetObjTMBeforeWSM(t);
        auto mat = to_float4x4(m);
        // cancel scale
        return mu::transform(extract_position(mat), extract_rotation(mat), mu::float3::one());
    }
    else {
        return to_float4x4(n->GetNodeTM(t));
    }
}

void msmaxContext::extractTransform(INode *n, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    auto mat = getGlobalMatrix(n, t);

    // handle parents
    if (auto parent = n->GetParentNode()) {
        auto pmat = getGlobalMatrix(parent, t);
        mat *= mu::invert(pmat);
    }

    pos = mu::extract_position(mat);
    rot = mu::extract_rotation(mat);
    scale = mu::extract_scale(mat);
    vis = (n->IsHidden() || !IsVisibleInHierarchy(n, t)) ? false : true;

    {
        auto *obj = GetBaseObject(n);
        if (IsCamera(obj) || IsLight(obj)) {
            static const auto cr = mu::rotate_y(180.0f * mu::DegToRad);
            rot *= cr;
        }
    }
}

void msmaxContext::extractTransform(TreeNode& n)
{
    auto& dst = *n.dst;
    auto t = GetTime();
    extractTransform(n.node, t, dst.position, dst.rotation, dst.scale, dst.visible);

    if (m_settings.bake_modifiers && m_settings.sc_flatten_hierarchy) {
        n.dst->assignMatrix(getGlobalMatrix(n.node, t));
    }
}

void msmaxContext::extractCameraData(GenCamera *cam, TimeValue t,
    bool& ortho, float& fov, float& near_plane, float& far_plane,
    float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift)
{
    float aspect = GetCOREInterface()->GetRendImageAspect();
    ortho = cam->IsOrtho();
    {
        float hfov = cam->GetFOV(t);
        // CameraObject::GetFOV() returns horizontal fov. we need vertical one.
        float vfov = 2.0f * std::atan(std::tan(hfov / 2.0f) / aspect);
        fov = vfov * mu::RadToDeg;
    }
    if (cam->GetManualClip()) {
        near_plane = cam->GetClipDist(t, 0);
        far_plane = cam->GetClipDist(t, 1);
    }
    else {
        near_plane = far_plane = 0.0f;
    }

    if (auto* pcam = dynamic_cast<MaxSDK::IPhysicalCamera*>(cam)) {
        float to_mm = (float)GetMasterScale(UNITS_MILLIMETERS);
        Interval interval;
        focal_length = pcam->GetEffectiveLensFocalLength(t, interval) * to_mm;
        float film_width = pcam->GetFilmWidth(t, interval) * to_mm;
        sensor_size.x = film_width;
        sensor_size.y = film_width / aspect;
        lens_shift = mu::float2::zero();
    }
    else {
        focal_length = 0.0f;
        sensor_size = mu::float2::zero();
        lens_shift = mu::float2::zero();
    }
}

void msmaxContext::extractLightData(GenLight *light, TimeValue t,
    ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle)
{
    switch (light->Type()) {
    case TSPOT_LIGHT:
    case FSPOT_LIGHT: // fall through
    {
        ltype = ms::Light::LightType::Spot;
        spot_angle = light->GetHotspot(t);
        break;
    }
    case DIR_LIGHT:
    case TDIR_LIGHT: // fall through
    case LightscapeLight::TARGET_POINT_TYPE:
    case LightscapeLight::TARGET_LINEAR_TYPE:
    case LightscapeLight::TARGET_AREA_TYPE:
    case LightscapeLight::TARGET_DISC_TYPE:
    case LightscapeLight::TARGET_SPHERE_TYPE:
    case LightscapeLight::TARGET_CYLINDER_TYPE:
    {
        ltype = ms::Light::LightType::Directional;
        break;
    }
    case OMNI_LIGHT:
    default: // fall through
    {
        ltype = ms::Light::LightType::Point;
        break;
    }
    }
    stype = light->GetShadow() != 0 ? ms::Light::ShadowType::Soft : ms::Light::ShadowType::None;

    (mu::float3&)color = to_float3(light->GetRGBColor(t));
    intensity = light->GetIntensity(t);
}


template<class T>
std::shared_ptr<T> msmaxContext::createEntity(TreeNode& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = n.path;
    dst.index = n.index;
    n.dst = ret;
    return ret;
}


ms::TransformPtr msmaxContext::exportTransform(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;

    extractTransform(n);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msmaxContext::exportInstance(TreeNode& n, ms::TransformPtr base)
{
    if (!base)
        return nullptr;

    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;

    extractTransform(n);
    dst.reference = base->path;
    m_entity_manager.add(ret);
    return ret;
}

ms::CameraPtr msmaxContext::exportCamera(TreeNode& n)
{
    auto ret = createEntity<ms::Camera>(n);
    auto& dst = *ret;
    extractTransform(n);
    extractCameraData((GenCamera*)n.baseobj, GetTime(),
        dst.is_ortho, dst.fov, dst.near_plane, dst.far_plane, dst.focal_length, dst.sensor_size, dst.lens_shift);
    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msmaxContext::exportLight(TreeNode& n)
{
    auto ret = createEntity<ms::Light>(n);
    auto& dst = *ret;
    extractTransform(n);
    extractLightData((GenLight*)n.baseobj, GetTime(),
        dst.light_type, dst.shadow_type, dst.color, dst.intensity, dst.spot_angle);
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

ms::MeshPtr msmaxContext::exportMesh(TreeNode& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    auto inode = n.node;
    auto& dst = *ret;
    extractTransform(n);

    // send mesh contents even if the node is hidden.

    Mesh *mesh = nullptr;
    TriObject *tri = nullptr;
    bool needs_delete = false;

    if (m_settings.sync_meshes) {
        tri = m_settings.bake_modifiers ?
            GetFinalMesh(inode, needs_delete) :
            GetSourceMesh(inode, needs_delete);
        if (tri) {
            if (needs_delete)
                m_tmp_triobj.push_back(tri);

            if (m_settings.use_render_meshes) {
                // get render mesh
                // todo: support multiple meshes
                BOOL del;
                NullView view;
                mesh = tri->GetRenderMesh(GetTime(), inode, view, del);
                if (del)
                    m_tmp_meshes.push_back(mesh);
            }
            else {
                // get viewport mesh
                mesh = &tri->GetMesh();
            }
            if (mesh)
                mesh->checkNormals(TRUE);
        }
    }
    if (!mesh)
        return ret;

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

void msmaxContext::doExtractMeshData(ms::Mesh &dst, INode *n, Mesh *mesh)
{
    auto t = GetTime();
    if (mesh) {
        if (!m_settings.bake_modifiers) {
            // handle pivot
            dst.refine_settings.flags.apply_local2world = 1;
            dst.refine_settings.local2world = getPivotMatrix(n);
        }
        else {
            // convert vertices
            //   (local space) -> world space -> local space without scale
            // to handle world space problem and complex scale composition
            // ( https://help.autodesk.com/view/3DSMAX/2016/ENU/?guid=__files_GUID_2E4E41D4_1B52_48C8_8ABA_3D3C9910CB2C_htm )
            if (IsInWorldSpace(n, t))
                dst.refine_settings.local2world = mu::invert(getGlobalMatrix(n, t));
            else
                dst.refine_settings.local2world = to_float4x4(n->GetObjTMAfterWSM(t)) * mu::invert(getGlobalMatrix(n, t));
            dst.refine_settings.flags.apply_local2world = 1;
        }

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
                    if (!channel.IsValidProgressiveMorphTargetIndex(ti))
                        continue;

                    dbs->frames.push_back(ms::BlendShapeFrameData::create());
                    auto& frame = *dbs->frames.back();
                    frame.weight = channel.GetProgressiveMorphWeight(ti);

                    // workaround.
                    if (frame.weight == 0.0f)
                        frame.weight = 100.0f;

                    // gen delta
                    frame.points.resize_discard(num_points);
                    for (int vi = 0; vi < num_points; ++vi)
                        frame.points[vi] = to_float3(channel.GetProgressiveMorphPoint(ti, vi)) - dst.points[vi];
                }
                if (!dbs->frames.empty()) {
                    dbs->name = mu::ToMBS(channel.GetName());
                    dbs->weight = channel.GetMorphWeight(t);
                    dst.blendshapes.push_back(dbs);
                }
            }
        }
    }

    {
        if (dst.normals.empty())
            dst.refine_settings.flags.gen_normals = 1;
        if (dst.tangents.empty())
            dst.refine_settings.flags.gen_tangents = 1;
        dst.refine_settings.flags.flip_faces = m_settings.flip_faces;
        dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
        dst.setupMeshDataFlags();
    }
}


bool msmaxContext::exportAnimations(INode *n, bool force)
{
    if (!n || !n->GetObjectRef())
        return false;

    auto it = m_anim_records.find(n);
    if (it != m_anim_records.end())
        return false;

    auto obj = GetBaseObject(n);
    ms::TransformAnimationPtr ret;
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
        extractor = &msmaxContext::extractMeshAnimation;
    }
    else {
        if (IsCamera(obj)) {
            exportAnimations(n->GetParentNode(), true);
            ret = ms::CameraAnimation::create();
            extractor = &msmaxContext::extractCameraAnimation;
        }
        else if (IsLight(obj)) {
            exportAnimations(n->GetParentNode(), true);
            ret = ms::LightAnimation::create();
            extractor = &msmaxContext::extractLightAnimation;
        }
        else {
            if (force) {
                exportAnimations(n->GetParentNode(), true);
                ret = ms::TransformAnimation::create();
                extractor = &msmaxContext::extractTransformAnimation;
            }
        }
    }

    if (ret) {
        m_animations.front()->addAnimation(ret);
        ret->path = GetPath(n);

        auto& rec = m_anim_records[n];
        rec.dst = ret;
        rec.node = n;
        rec.obj = obj;
        rec.extractor = extractor;
    }
    return ret != nullptr;
}

void msmaxContext::extractTransformAnimation(ms::TransformAnimation& dst_, INode *src, Object *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    bool vis;
    extractTransform(src, m_current_time_tick, pos, rot, scale, vis);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    dst.visible.push_back({ t, vis });
}

void msmaxContext::extractCameraAnimation(ms::TransformAnimation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_anim_time;
    auto& dst = (ms::CameraAnimation&)dst_;

    bool ortho;
    float fov, near_plane, far_plane, focal_length;
    mu::float2 sensor_size, lens_shift;
    extractCameraData((GenCamera*)obj, m_current_time_tick, ortho, fov, near_plane, far_plane, focal_length, sensor_size, lens_shift);

    dst.fov.push_back({ t, fov });
    dst.near_plane.push_back({ t, near_plane });
    dst.far_plane.push_back({ t, far_plane });
    if (focal_length > 0.0f) {
        dst.focal_length.push_back({ t, focal_length });
        dst.sensor_size.push_back({ t, sensor_size });
        dst.lens_shift.push_back({ t, lens_shift });
    }
}

void msmaxContext::extractLightAnimation(ms::TransformAnimation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_anim_time;
    auto& dst = (ms::LightAnimation&)dst_;

    ms::Light::LightType ltype;
    ms::Light::ShadowType stype;
    mu::float4 color;
    float intensity, spot_angle;
    extractLightData((GenLight*)obj, m_current_time_tick, ltype, stype, color, intensity, spot_angle);

    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (ltype == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void msmaxContext::extractMeshAnimation(ms::TransformAnimation& dst_, INode *src, Object *obj)
{
    extractTransformAnimation(dst_, src, obj);

    float t = m_anim_time;
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
                dst.getBlendshapeCurve(name).push_back({ t, channel.GetMorphWeight(m_current_time_tick) });
            }
        }
    }
}

void msmaxContext::addDeferredCall(const std::function<void()>& c)
{
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_deferred_calls.push_back(c);
    }
    FeedDeferredCalls();
}

void msmaxContext::feedDeferredCalls()
{
    std::unique_lock<std::mutex> l(m_mutex);
    for (auto& c : m_deferred_calls)
        c();
    m_deferred_calls.clear();
}


bool msmaxSendScene(msmaxContext::SendTarget target, msmaxContext::SendScope scope)
{
    auto& ctx = msmaxGetContext();
    if (!ctx.isServerAvailable()) {
        ctx.logInfo("MeshSync: Server not available. %s", ctx.getErrorMessage().c_str());
        return false;
    }

    auto body = [&ctx, target, scope]() {
        if (target == msmaxContext::SendTarget::Objects) {
            ctx.wait();
            ctx.sendObjects(scope, true);
        }
        else if (target == msmaxContext::SendTarget::Materials) {
            ctx.wait();
            ctx.sendMaterials(true);
        }
        else if (target == msmaxContext::SendTarget::Animations) {
            ctx.wait();
            ctx.sendAnimations(scope);
        }
        else if (target == msmaxContext::SendTarget::Everything) {
            ctx.wait();
            ctx.sendMaterials(true);
            ctx.wait();
            ctx.sendObjects(scope, true);
            ctx.wait();
            ctx.sendAnimations(scope);
        }
    };
    ctx.addDeferredCall(body);
    return true;
}

bool msmaxExportCache(msmaxContext::SendScope scope, bool all_frames)
{
    auto *ifs = GetCOREInterface8();

    MSTR filename = ifs->GetCurFileName();
    {
        auto len = filename.Length();
        if (len == 0) {
            filename = L"Untitles.sc";
        }
        else {
            // replace extention (.max -> .sc)
            int ext_pos = 0;
            for (int i = 0; i < len; ++i) {
                ext_pos = i;
                if (filename[i] == L'.')
                    break;
            }
            filename.Resize(ext_pos);
            filename += L".sc";
        }
    }

    MSTR dir = L"";

    int filter_index = 0;
    FilterList filter_list;
    filter_list.Append(_M("Scene cache files(*.sc)"));
    filter_list.Append(_M("*.sc"));
    filter_list.SetFilterIndex(filter_index);

    if (ifs->DoMaxSaveAsDialog(ifs->GetMAXHWnd(), L"Export Scene Cache", filename, dir, filter_list)) {
        auto& ctx = msmaxGetContext();
        return ctx.writeCache(scope, all_frames, ms::ToMBS(filename));
    }
    return false;
}
