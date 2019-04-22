#include "pch.h"
#include "msmobuDevice.h"
#include "msmobuUtils.h"


FBDeviceImplementation(msmobuDevice);
FBRegisterDevice("msmbDevice", msmobuDevice, "UnityMeshSync", "UnityMeshSync for MotionBuilder", FB_DEFAULT_SDK_ICON);

ms::Identifier msmobuDevice::NodeRecord::getIdentifier() const
{
    return { path,id };
}

bool msmobuDevice::FBCreate()
{
    FBSystem::TheOne().Scene->OnChange.Add(this, (FBCallback)&msmobuDevice::onSceneChange);
    FBSystem::TheOne().OnConnectionDataNotify.Add(this, (FBCallback)&msmobuDevice::onDataUpdate);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&msmobuDevice::onRender);
    FBEvaluateManager::TheOne().OnSynchronizationEvent.Add(this, (FBCallback)&msmobuDevice::onSynchronization);
    return true;
}

void msmobuDevice::FBDestroy()
{
    wait();

    FBSystem::TheOne().Scene->OnChange.Remove(this, (FBCallback)&msmobuDevice::onSceneChange);
    FBSystem::TheOne().OnConnectionDataNotify.Remove(this, (FBCallback)&msmobuDevice::onDataUpdate);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&msmobuDevice::onRender);
    FBEvaluateManager::TheOne().OnSynchronizationEvent.Remove(this, (FBCallback)&msmobuDevice::onSynchronization);
}

bool msmobuDevice::DeviceOperation(kDeviceOperations pOperation)
{
    return false;
}

void msmobuDevice::DeviceTransportNotify(kTransportMode pMode, FBTime pTime, FBTime pSystem)
{
    if (m_settings.auto_sync)
        m_data_updated = true;
}

void msmobuDevice::onSceneChange(HIRegister pCaller, HKEventBase pEvent)
{
    FBEventSceneChange SceneChangeEvent = pEvent;
    FBSceneChangeType type = SceneChangeEvent.Type;
    switch (type)
    {
    case kFBSceneChangeDestroy:
    case kFBSceneChangeAttach:
    case kFBSceneChangeDetach:
    case kFBSceneChangeAddChild:
    case kFBSceneChangeRemoveChild:
    case kFBSceneChangeRenamed:
    case kFBSceneChangeRenamedPrefix:
    case kFBSceneChangeRenamedUnique:
    case kFBSceneChangeRenamedUniquePrefix:
    case kFBSceneChangeLoadEnd:
    case kFBSceneChangeClearEnd:
    case kFBSceneChangeTransactionEnd:
    case kFBSceneChangeMergeTransactionEnd:
    case kFBSceneChangeChangeName:
    case kFBSceneChangeChangedName:
        if (type == kFBSceneChangeLoadEnd || type == kFBSceneChangeAddChild)
            m_dirty_meshes = m_dirty_textures = true;
        m_data_updated = true;
        break;

    default:
        break;
    }
}

void msmobuDevice::onDataUpdate(HIRegister pCaller, HKEventBase pEvent)
{
    m_data_updated = true;
    if (m_settings.bake_deformars)
        m_dirty_meshes = true;
}

void msmobuDevice::onRender(HIRegister pCaller, HKEventBase pEvent)
{
    // note: mocap devices seem don't trigger scene change events.
    //       so, always set m_pending true on render when auto sync is enabled.
    //       obviously this wastes CPU time, but I couldn't find a better way... (issue #47)
    if (m_settings.auto_sync /*&& m_data_updated*/) {
        m_pending = true;
        if (m_settings.bake_deformars)
            m_dirty_meshes = true;
    }
}

void msmobuDevice::onSynchronization(HIRegister pCaller, HKEventBase pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    FBGlobalEvalCallbackTiming timing = lFBEvent.GetTiming();
    if (timing == kFBGlobalEvalCallbackSyn) {
        if (m_settings.auto_sync)
            update();
    }
}

msmobuSettings& msmobuDevice::getSettings()
{
    return m_settings;
}

void msmobuDevice::logInfo(const char *format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    FBTraceWithLevel(kFBNORMAL_TRACE, buf);
    va_end(args);
}
void msmobuDevice::logError(const char *format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    FBTraceWithLevel(kFBCRITICAL_TRACE, buf);
    va_end(args);
}

bool msmobuDevice::isServerAvailable()
{
    m_sender.client_settings = m_settings.client_settings;
    return m_sender.isServerAvaileble();
}
const std::string& msmobuDevice::getErrorMessage()
{
    return m_sender.getErrorMessage();
}


void msmobuDevice::wait()
{
    m_sender.wait();
}

void msmobuDevice::update()
{
    if (!m_pending)
        return;
    sendObjects(false);
}

void msmobuDevice::kickAsyncSend()
{
    // process extract tasks
    if (!m_extract_tasks.empty()) {
        if (m_settings.parallel_extraction) {
            mu::parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](ExtractTasks::value_type& task) {
                task();
            });
        }
        else {
            for (auto& task : m_extract_tasks) {
                task();
            }
        }
        m_extract_tasks.clear();
    }


    m_sender.on_prepare = [this]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor;

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


bool msmobuDevice::sendMaterials(bool dirty_all)
{
    if (m_sender.isSending()) {
        return false;
    }

    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(dirty_all);
    exportMaterials();

    // send
    kickAsyncSend();
    return true;
}

bool msmobuDevice::sendObjects(bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending = true;
        return false;
    }
    m_data_updated = m_pending = false;

    if (dirty_all) {
        m_dirty_meshes = true;
        m_dirty_textures = true;
    }
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy

    if (m_settings.sync_meshes)
        exportMaterials();

    // export nodes
    int num_exported = 0;
    EnumerateAllNodes([this, &num_exported](FBModel* node) {
        if (exportObject(node, true))
            ++num_exported;
    });

    // check deleted objects
    for (auto it = m_node_records.begin(); it != m_node_records.end(); /**/) {
        if (!it->second.exist) {
            m_entity_manager.erase(it->second.getIdentifier());
            m_node_records.erase(it++);
        }
        else {
            ++it;
        }
    }

    // clear state
    for (auto& kvp : m_node_records) {
        kvp.second.dst = nullptr;
        kvp.second.exist = false;
    }
    m_dirty_meshes = m_dirty_textures = false;

    // send
    if (num_exported || !m_entity_manager.getDeleted().empty()) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

ms::TransformPtr msmobuDevice::exportObject(FBModel* src, bool parent, bool tip)
{
    if (!src)
        return nullptr;

    auto& rec = m_node_records[src];
    if (rec.dst)
        return rec.dst;

    {
        // check rename
        if (rec.name.empty()) {
            rec.src = src;
            rec.name = GetName(src);
            rec.path = GetPath(src);
            rec.index = ++m_node_index_seed;
        }
        else if (rec.name != GetName(src)) {
            // renamed
            m_entity_manager.erase(rec.getIdentifier());
            rec.name = GetName(src);
            rec.path = GetPath(src);
        }
        rec.exist = true;
    }

    ms::TransformPtr ret;
    auto handle_parent = [&]() {
        if (parent)
            exportObject(src->Parent, parent, false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        ret = exportTransform(rec);
    };

    if (IsCamera(src)) { // camera
        if (m_settings.sync_cameras) {
            handle_parent();
            ret = exportCamera(rec);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (IsLight(src)) { // light
        if (m_settings.sync_lights) {
            handle_parent();
            ret = exportLight(rec);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (IsMesh(src)) { // mesh
        if (m_settings.sync_bones && !m_settings.bake_deformars) {
            EachBones(src, [this](FBModel *bone) {
                exportObject(bone, true);
            });
        }
        if (m_settings.sync_meshes || m_settings.sync_blendshapes) {
            handle_parent();
            if (m_settings.sync_meshes && m_dirty_meshes)
                ret = exportMesh(rec);
            else
                ret = exportBlendshapeWeights(rec);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else {
        handle_transform();
    }

    return ret;
}


static void ExtractTransformData(FBModel *src, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    FBMatrix tmp;
    src->GetMatrix(tmp, kModelTransformation, true, nullptr);
    auto trs = to_float4x4(tmp);

    if (src->Parent) {
        src->Parent->GetMatrix(tmp, kModelTransformation, true, nullptr);
        trs *= mu::invert(to_float4x4(tmp));
    }

    pos = extract_position(trs);
    rot = extract_rotation(trs);
    scale = extract_scale(trs);
    vis = IsVisibleInHierarchy(src);

    if (IsCamera(src))
        rot *= mu::rotate_y(90.0f * mu::DegToRad);
    else if (IsLight(src))
        rot *= mu::rotate_x(90.0f * mu::DegToRad);
}

static void ExtractCameraData(FBCamera* src, bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift)
{
    ortho = src->Type == kFBCameraTypeOrthogonal;
    near_plane = (float)src->NearPlaneDistance;
    far_plane = (float)src->FarPlaneDistance;
    fov = (float)src->FieldOfViewY;

    focal_length = (float)src->FocalLength;
    sensor_size.x = (float)(src->FilmSizeWidth * mu::InchToMillimeter_d);
    sensor_size.y = (float)(src->FilmSizeHeight * mu::InchToMillimeter_d);
    lens_shift.x = (float)src->OpticalCenterX;
    lens_shift.y = (float)src->OpticalCenterY;
}

static void ExtractLightData(FBLight* src, ms::Light::LightType& type, mu::float4& color, float& intensity, float& spot_angle)
{
    FBLightType light_type = src->LightType;
    if (light_type == kFBLightTypePoint) {
        type = ms::Light::LightType::Point;
    }
    else if (light_type == kFBLightTypeInfinite) {
        type = ms::Light::LightType::Directional;
    }
    else if (light_type == kFBLightTypeSpot) {
        type = ms::Light::LightType::Spot;
        spot_angle = (float)src->OuterAngle;
    }
    else if (light_type == kFBLightTypeArea) {
        type = ms::Light::LightType::Area;
    }

    color = to_float4(src->DiffuseColor);
    intensity = (float)src->Intensity * 0.01f;
}

// Body: [](const char *name, double value) -> void
template<class Body>
static inline void EachAnimationNVP(FBModel *src, const Body& body)
{
    FBAnimationNode *anode = src->AnimationNode;
    if (anode) {
        Each(anode->Nodes, [&body](FBAnimationNode *n) {
            // FBAnimationNode may have array of double data. (translation: 3 elements, blendshape weight: 1 element, etc)
            // and ReadData() assume dst has enough space. so we must be extremely careful!
            const char *name = n->Name;
            int c = n->GetDataDoubleArrayCount();
            if (c == 1) {
                double value;
                n->ReadData(&value);
                body(name, value);
            }
        });
    }
}


template<class T>
inline std::shared_ptr<T> msmobuDevice::createEntity(NodeRecord& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = n.path;
    dst.index = n.index;
    n.dst = ret;
    return ret;
}

ms::TransformPtr msmobuDevice::exportTransform(NodeRecord& n)
{
    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;
    auto src = n.src;

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);

    m_entity_manager.add(ret);
    return ret;
}

ms::CameraPtr msmobuDevice::exportCamera(NodeRecord& n)
{
    auto ret = createEntity<ms::Camera>(n);
    auto& dst = *ret;
    auto src = static_cast<FBCamera*>(n.src);

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
    ExtractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov,
        dst.focal_length, dst.sensor_size, dst.lens_shift);

    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msmobuDevice::exportLight(NodeRecord& n)
{
    auto ret = createEntity<ms::Light>(n);
    auto& dst = *ret;
    auto src = static_cast<FBLight*>(n.src);

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
    ExtractLightData(src, dst.light_type, dst.color, dst.intensity, dst.spot_angle);

    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msmobuDevice::exportBlendshapeWeights(NodeRecord& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    auto& dst = *ret;
    auto src = n.src;

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);

    // blendshape weights
    if (FBGeometry *geom = src->Geometry) {
        int num_shapes = geom->ShapeGetCount();
        if (num_shapes) {
            std::map<std::string, float> weight_table;
            EachAnimationNVP(src, [&weight_table](const char *name, double value) {
                weight_table[name] = (float)value;
            });

            for (int si = 0; si < num_shapes; ++si) {
                auto name = geom->ShapeGetName(si);
                auto it = weight_table.find(name);
                if (it != weight_table.end()) {
                    auto bsd = ms::BlendShapeData::create();
                    dst.blendshapes.push_back(bsd);
                    bsd->name = it->first;
                    bsd->weight = it->second;
                }
            }
        }
    }
    dst.setupFlags();

    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msmobuDevice::exportMesh(NodeRecord& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    auto& dst = *ret;
    auto src = n.src;

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);

    m_extract_tasks.push_back([this, ret, &dst, src]() {
        doExtractMesh(dst, src);
        m_entity_manager.add(ret);
    });
    return ret;
}

void msmobuDevice::doExtractMesh(ms::Mesh& dst, FBModel * src)
{
    FBModelVertexData *vd = src->ModelVertexData;
    int num_vertices = vd->GetVertexCount();

    // points
    {
        auto points = (const FBVertex*)vd->GetVertexArray(kFBGeometryArrayID_Point, m_settings.bake_deformars);
        if (!points)
            return;

        dst.points.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi)
            dst.points[vi] = to_float3(points[vi]);
    }

    // normals
    {
        auto normals = (const FBNormal*)vd->GetVertexArray(kFBGeometryArrayID_Normal, m_settings.bake_deformars);
        if (normals) {
            dst.normals.resize_discard(num_vertices);
            for (int vi = 0; vi < num_vertices; ++vi)
                dst.normals[vi] = to_float3(normals[vi]);
        }
        else {
            dst.refine_settings.flags.gen_normals = 1;
        }
    }

    // uv
    {
        auto uv_ = vd->GetUVSetArray();
        if (uv_) {
            auto type = vd->GetUVSetArrayFormat();
            if (type == kFBGeometryArrayElementType_Float2) {
                auto uv = (const mu::float2*)uv_;
                dst.uv0.assign(uv, uv + num_vertices);
                dst.refine_settings.flags.gen_tangents = 1;
            }
            else if (type == kFBGeometryArrayElementType_Float3) {
                auto uv = (const mu::float3*)uv_;
                dst.uv0.resize_discard(num_vertices);
                for (int vi = 0; vi < num_vertices; ++vi)
                    dst.uv0[vi] = (mu::float2&)uv[vi];
                dst.refine_settings.flags.gen_tangents = 1;
            }
            else if (type == kFBGeometryArrayElementType_Float4) {
                auto uv = (const mu::float4*)uv_;
                dst.uv0.resize_discard(num_vertices);
                for (int vi = 0; vi < num_vertices; ++vi)
                    dst.uv0[vi] = (mu::float2&)uv[vi];
                dst.refine_settings.flags.gen_tangents = 1;
            }
        }
    }

    // colors
    if (auto colors_ = vd->GetVertexArray(kFBGeometryArrayID_Color, m_settings.bake_deformars)) {
        auto type = vd->GetVertexArrayType(kFBGeometryArrayID_Color);
        if (type == kFBGeometryArrayElementType_Float4) {
            auto colors = (const mu::float4*)colors_;
            dst.colors.assign(colors, colors + num_vertices);
        }
        else if (type == kFBGeometryArrayElementType_Float3) {
            dst.colors.resize_discard(num_vertices);
            auto colors = (const mu::float3*)colors_;
            for (int vi = 0; vi < num_vertices; ++vi) {
                auto t = colors[vi];
                dst.colors[vi] = { t[0], t[1], t[2], 1.0f };
            }
        }
    }

    if (!m_settings.bake_deformars && m_settings.sync_bones) {
        // skin cluster
        if (FBCluster *cluster = src->Cluster) {

            // MotionBuilder omits weight data if there are vertices with identical position.
            // so generate vertex reference map.
            struct Ref { int vi, ri; };
            RawVector<Ref> brefmap;
            brefmap.reserve(num_vertices / 4);
            for (int vi = num_vertices - 1; vi >= 0; --vi) {
                auto beg = dst.points.begin();
                auto end = beg + vi;
                auto it = std::find(beg, end, dst.points[vi]);
                if (it != end)
                    brefmap.push_back({ vi, (int)std::distance(beg, it) });
                else
                    break;
            }


            int num_links = cluster->LinkGetCount();
            for (int li = 0; li < num_links; ++li) {
                ClusterScope scope(cluster, li);

                int n = cluster->VertexGetCount();
                if (n == 0)
                    continue;

                auto bd = ms::BoneData::create();
                dst.bones.push_back(bd);

                auto bone = cluster->LinkGetModel(li);
                bd->path = GetPath(bone);

                // root bone
                if (li == 0) {
                    auto root = bone;
                    while (IsBone(root->Parent))
                        root = root->Parent;
                    dst.root_bone = GetPath(root);
                }

                // bindpose
                {
                    // should consider rotation order?
                    //FBModelRotationOrder order = bone->RotationOrder;

                    FBVector3d t, r, s;
                    cluster->VertexGetTransform(t, r, s);
                    mu::quatf q = mu::invert(mu::rotate_xyz(to_float3(r) * mu::DegToRad));
                    bd->bindpose = mu::transform(to_float3(t), q, to_float3(s));
                }

                // weights
                bd->weights.resize_zeroclear(num_vertices);
                for (int i = 0; i < n; ++i) {
                    int vi = cluster->VertexGetNumber(i);
                    float w = (float)cluster->VertexGetWeight(i);
                    bd->weights[vi] = w;
                }
                for (auto& rel : brefmap)
                    bd->weights[rel.vi] = bd->weights[rel.ri];
            }
        }
    }

    if (!m_settings.bake_deformars && m_settings.sync_blendshapes) {
        // blendshapes
        if (FBGeometry *geom = src->Geometry) {
            int num_shapes = geom->ShapeGetCount();
            if (num_shapes) {
                std::map<std::string, float> weight_table;
                EachAnimationNVP(src, [&weight_table](const char *name, double value) {
                    weight_table[name] = (float)value;
                });

                RawVector<mu::float3> tmp_points, tmp_normals;
                for (int si = 0; si < num_shapes; ++si) {
                    auto bsd = ms::BlendShapeData::create();
                    dst.blendshapes.push_back(bsd);
                    bsd->name = geom->ShapeGetName(si);
                    {
                        auto it = weight_table.find(bsd->name);
                        if (it != weight_table.end())
                            bsd->weight = it->second;
                    }

                    auto bsfd = ms::BlendShapeFrameData::create();
                    bsfd->weight = 100.0f;
                    bsd->frames.push_back(bsfd);

                    tmp_points.resize_zeroclear(dst.points.size());
                    tmp_normals.resize_zeroclear(dst.normals.size());

                    int num_points = geom->ShapeGetDiffPointCount(si);
                    if (!tmp_normals.empty()) {
                        for (int pi = 0; pi < num_points; ++pi) {
                            int vi = 0; // zero initialization is needed or ShapeGetDiffPoint() fails
                            FBVertex p, n;
                            if (geom->ShapeGetDiffPoint(si, pi, vi, p, n)) {
                                tmp_points[vi] = to_float3(p);
                                tmp_normals[vi] = to_float3(n);
                            }
                        }
                        bsfd->points = std::move(tmp_points);
                        bsfd->normals = std::move(tmp_normals);
                    }
                    else {
                        for (int pi = 0; pi < num_points; ++pi) {
                            int vi = 0; // zero initialization is needed or ShapeGetDiffPoint() fails
                            FBVertex p;
                            if (geom->ShapeGetDiffPoint(si, pi, vi, p)) {
                                tmp_points[vi] = to_float3(p);
                            }
                        }
                        bsfd->points = std::move(tmp_points);
                    }
                }
            }
        }
    }

    // enumerate subpatches ("submeshes" in Unity's term) and generate indices
    int num_subpatches = vd->GetSubPatchCount();
    auto indices = (const int*)vd->GetIndexArray();
    for (int spi = 0; spi < num_subpatches; ++spi) {
        // note: quad subpatches should be ignored.
        // FBModelVertexData contains both triangulated subpatches and original quad ones.
        // so if import both, excessive unnecessary triangles will be generated.
        // (possibly we should handle kFBGeometry_POINTS and kFBGeometry_LINES. but leave it on at this point)
        if (vd->GetSubPatchPrimitiveType(spi) != kFBGeometry_TRIANGLES)
            continue;

        int offset = vd->GetSubPatchIndexOffset(spi);
        int count = vd->GetSubPatchIndexSize(spi);
        int mid = m_material_records[vd->GetSubPatchMaterial(spi)].id;
        auto idx_begin = indices + offset;
        auto idx_end = idx_begin + count;

        int ngon = 3;
        int prim_count = count / ngon;

        dst.indices.insert(dst.indices.end(), idx_begin, idx_end);
        dst.counts.resize(dst.counts.size() + prim_count, ngon);
        dst.material_ids.resize(dst.material_ids.size() + prim_count, mid);
    }

    dst.refine_settings.flags.flip_faces = 1;
    dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
    dst.setupFlags();
}


int msmobuDevice::exportTexture(FBTexture* src, FBMaterialTextureType type)
{
    if (!src)
        return -1;

    FBVideoClip* video = FBCast<FBVideoClip>(src->Video);
    if (!video)
        return -1;

    if (ms::FileExists(video->Filename)) {
        ms::TextureType textype = ms::TextureType::Default;
        if (type == kFBMaterialTextureNormalMap)
            textype = ms::TextureType::NormalMap;

        return m_texture_manager.addFile(video->Filename.AsString(), textype);
    }
    return -1;
}

bool msmobuDevice::exportMaterial(FBMaterial* src, int index)
{
    if (!src)
        return false;

    auto& rec = m_material_records[src];
    if (rec.dst)
        return rec.id; // already exported

    auto dst = ms::Material::create();
    rec.dst = dst.get();
    if (rec.id == ms::InvalidID)
        rec.id = m_material_ids.getID(src);

    dst->id = rec.id;
    dst->index = index;
    dst->name = src->LongName;

    auto& stdmat = ms::AsStandardMaterial(*dst);
    stdmat.setColor(to_float4(src->Diffuse));

    auto emissive = to_float4(src->Emissive);
    if ((mu::float3&)emissive != mu::float3::zero())
        stdmat.setEmissionColor(emissive);

    if (m_settings.sync_textures && m_dirty_textures) {
        stdmat.setColorMap(exportTexture(src->GetTexture(kFBMaterialTextureDiffuse), kFBMaterialTextureDiffuse));
        stdmat.setEmissionMap(exportTexture(src->GetTexture(kFBMaterialTextureEmissive), kFBMaterialTextureEmissive));
        stdmat.setBumpMap(exportTexture(src->GetTexture(kFBMaterialTextureNormalMap), kFBMaterialTextureNormalMap));
    }
    m_material_manager.add(dst);
    return true;
}

bool msmobuDevice::exportMaterials()
{
    int num_exported = 0;
    auto& materials = FBSystem::TheOne().Scene->Materials;
    const int num_materials = materials.GetCount();
    for (int mi = 0; mi < num_materials; ++mi) {
        if (exportMaterial(materials[mi], mi))
            ++num_exported;
    }

    for (auto& kvp : m_texture_records)
        kvp.second.dst = nullptr;
    for (auto& kvp : m_material_records)
        kvp.second.dst = nullptr;

    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
    return num_exported > 0;
}


bool msmobuDevice::sendAnimations()
{
    if (m_sender.isSending())
        return false;

    if (exportAnimations())
        kickAsyncSend();
    return true;
}

bool msmobuDevice::exportAnimations()
{
    auto& system = FBSystem::TheOne();
    FBPlayerControl control;

    // create default clip
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create());

    // gather models
    int num_animations = 0;
    EnumerateAllNodes([this, &num_animations](FBModel *node) {
        if (exportAnimation(node, false))
            ++num_animations;
    });
    if (num_animations == 0)
        return false;


    FBTime time_current = system.LocalTime;
    double time_begin, time_end;
    std::tie(time_begin, time_end) = GetTimeRange(system.CurrentTake);
    double interval = 1.0 / std::max(m_settings.animation_sps, 0.01f);

    int reserve_size = int((time_end - time_begin) / interval) + 1;
    for (auto& kvp : m_anim_records) {
        kvp.second.dst->reserve(reserve_size);
    }

    // advance frame and record
    for (double t = time_begin;;) {
        FBTime fbt;
        fbt.SetSecondDouble(t);
        control.Goto(fbt);
        m_anim_time = (float)(t - time_begin) * m_settings.animation_time_scale;
        for (auto& kvp : m_anim_records)
            kvp.second(this);

        if (t >= time_end)
            break;
        else
            t = std::min(t + interval, time_end);
    }

    // cleanup
    m_anim_records.clear();
    control.Goto(time_current);

    if (m_settings.keyframe_reduction) {
        // keyframe reduction
        for (auto& clip : m_animations)
            clip->reduction(m_settings.keep_flat_curves);

        // erase empty clip
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
            m_animations.end());
    }

    return !m_animations.empty();
}

bool msmobuDevice::exportAnimation(FBModel *src, bool force)
{
    if (!src || m_anim_records.find(src) != m_anim_records.end())
        return 0;

    ms::TransformAnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (IsCamera(src)) { // camera
        exportAnimation(src->Parent, true);
        dst = ms::CameraAnimation::create();
        extractor = &msmobuDevice::extractCameraAnimation;
    }
    else if (IsLight(src)) { // light
        exportAnimation(src->Parent, true);
        dst = ms::LightAnimation::create();
        extractor = &msmobuDevice::extractLightAnimation;
    }
    else if (IsMesh(src)) { // mesh
        EachBones(src, [this](FBModel *bone) {
            exportAnimation(bone, true);
        });
        exportAnimation(src->Parent, true);
        dst = ms::MeshAnimation::create();
        extractor = &msmobuDevice::extractMeshAnimation;
    }
    else if (force) { // other
        exportAnimation(src->Parent, true);
        dst = ms::TransformAnimation::create();
        extractor = &msmobuDevice::extractTransformAnimation;
    }

    if (dst) {
        auto& rec = m_anim_records[src];
        rec.src = src;
        rec.dst = dst;
        rec.extractor = extractor;
        m_animations.front()->addAnimation(dst);
        return true;
    }
    else {
        return false;
    }
}

void msmobuDevice::extractTransformAnimation(ms::TransformAnimation& dst_, FBModel* src)
{
    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    bool vis = true;
    ExtractTransformData(src, pos, rot, scale, vis);

    float t = m_anim_time;
    auto& dst = (ms::TransformAnimation&)dst_;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    dst.visible.push_back({ t, vis });

    dst.path = GetPath(src);
}

void msmobuDevice::extractCameraAnimation(ms::TransformAnimation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::CameraAnimation&>(dst_);

    bool ortho;
    float near_plane, far_plane, fov, focal_length;
    mu::float2 sensor_size, lens_shift;
    ExtractCameraData(static_cast<FBCamera*>(src), ortho, near_plane, far_plane, fov, focal_length, sensor_size, lens_shift);

    float t = m_anim_time;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });

    dst.focal_length.push_back({ t, focal_length });
    dst.sensor_size.push_back({ t, sensor_size });
    dst.lens_shift.push_back({ t, lens_shift });
}

void msmobuDevice::extractLightAnimation(ms::TransformAnimation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::LightAnimation&>(dst_);

    ms::Light::LightType type;
    mu::float4 color;
    float intensity;
    float spot_angle;
    ExtractLightData(static_cast<FBLight*>(src), type, color, intensity, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (type == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void msmobuDevice::extractMeshAnimation(ms::TransformAnimation & dst_, FBModel * src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::MeshAnimation&>(dst_);

    // blendshape animations
    if (FBGeometry *geom = src->Geometry) {
        int num_shapes = geom->ShapeGetCount();
        if (num_shapes) {
            float t = m_anim_time;
            EachAnimationNVP(src, [&dst, t](const char *name, double value) {
                dst.getBlendshapeCurve(name).push_back({ t, (float)value });
            });
        }
    }
}

void msmobuDevice::AnimationRecord::operator()(msmobuDevice *_this)
{
    (_this->*extractor)(*dst, src);
}
