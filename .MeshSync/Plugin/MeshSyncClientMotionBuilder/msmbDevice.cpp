#include "pch.h"
#include "msmbDevice.h"
#include "msmbUtils.h"


FBDeviceImplementation(msmbDevice);
FBRegisterDevice("msmbDevice", msmbDevice, "UnityMeshSync", "UnityMeshSync for MotionBuilder", FB_DEFAULT_SDK_ICON);

ms::Identifier msmbDevice::NodeRecord::getIdentifier() const
{
    return { path,id };
}

bool msmbDevice::FBCreate()
{
    FBSystem::TheOne().Scene->OnChange.Add(this, (FBCallback)&msmbDevice::onSceneChange);
    FBSystem::TheOne().OnConnectionDataNotify.Add(this, (FBCallback)&msmbDevice::onDataUpdate);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&msmbDevice::onRender);
    FBEvaluateManager::TheOne().OnSynchronizationEvent.Add(this, (FBCallback)&msmbDevice::onSynchronization);
    return true;
}

void msmbDevice::FBDestroy()
{
    m_sender.wait();

    FBSystem::TheOne().Scene->OnChange.Remove(this, (FBCallback)&msmbDevice::onSceneChange);
    FBSystem::TheOne().OnConnectionDataNotify.Remove(this, (FBCallback)&msmbDevice::onDataUpdate);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&msmbDevice::onRender);
    FBEvaluateManager::TheOne().OnSynchronizationEvent.Remove(this, (FBCallback)&msmbDevice::onSynchronization);
}

bool msmbDevice::DeviceOperation(kDeviceOperations pOperation)
{
    return false;
}

void msmbDevice::DeviceTransportNotify(kTransportMode pMode, FBTime pTime, FBTime pSystem)
{
    if (auto_sync)
        m_data_updated = true;
}

void msmbDevice::onSceneChange(HIRegister pCaller, HKEventBase pEvent)
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
        if (type == kFBSceneChangeLoadEnd ||
            type == kFBSceneChangeAddChild)
            m_dirty_meshes = m_dirty_textures = true;
        m_data_updated = true;
        break;

    default:
        break;
    }
}

void msmbDevice::onDataUpdate(HIRegister pCaller, HKEventBase pEvent)
{
    m_data_updated = true;
    if (bake_deformars)
        m_dirty_meshes = true;
}

void msmbDevice::onRender(HIRegister pCaller, HKEventBase pEvent)
{
    // note: mocap devices seem doesn't trigger scene change events.
    //       so, always set m_pending true on render when auto sync is enabled.
    //       obviously this wastes CPU time, but I couldn't find a better way... (issue #47)
    if (auto_sync /*&& m_data_updated*/)
        m_pending = true;
}

void msmbDevice::onSynchronization(HIRegister pCaller, HKEventBase pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    FBGlobalEvalCallbackTiming timing = lFBEvent.GetTiming();
    if (timing == kFBGlobalEvalCallbackSyn) {
        if (auto_sync)
            update();
    }
}

void msmbDevice::update()
{
    if (!m_pending)
        return;
    sendScene(false);
}

void msmbDevice::kickAsyncSend()
{
    // process extract tasks
    if (!m_extract_tasks.empty()) {
        if (parallel_extraction) {
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
        t.client_settings = client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = scale_factor;

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


bool msmbDevice::sendScene(bool force_all)
{
    if (force_all) {
        m_dirty_meshes = m_dirty_textures = true;
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    m_data_updated = m_pending = false;
    if (m_sender.isSending()) {
        m_pending = true;
        return false;
    }

    if (sync_meshes)
        exportMaterials();

    // export nodes
    int num_exported = 0;
    EnumerateAllNodes([this, &num_exported](FBModel* node) {
        if (exportObject(node, false))
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

ms::TransformPtr msmbDevice::exportObject(FBModel* src, bool force)
{
    if (!src)
        return nullptr;

    auto& rec = m_node_records[src];
    if (rec.dst)
        return rec.dst;

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

    ms::TransformPtr ret;
    if (IsCamera(src)) { // camera
        if (sync_cameras) {
            exportObject(src->Parent, true);
            ret = exportCamera(rec);
        }
    }
    else if (IsLight(src)) { // light
        if (sync_lights) {
            exportObject(src->Parent, true);
            ret = exportLight(rec);
        }
    }
    else if (IsMesh(src)) { // mesh
        if (sync_bones && !bake_deformars) {
            EachBones(src, [this](FBModel *bone) {
                exportObject(bone, true);
            });
        }
        if (sync_meshes) {
            exportObject(src->Parent, true);
            if (m_dirty_meshes)
                ret = exportMesh(rec);
            else
                ret = exportMeshSimple(rec);
        }
    }
    else if (force) {
        exportObject(src->Parent, true);
        ret = exporttTransform(rec);
    }

    return ret;
}


static void ExtractTransformData(FBModel* src, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
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
    vis = src->Visibility;
}

static void ExtractCameraData(FBCamera* src, bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance)
{
    ortho = src->Type == kFBCameraTypeOrthogonal;
    near_plane = (float)src->NearPlaneDistance;
    far_plane = (float)src->FarPlaneDistance;
    fov = (float)src->FieldOfViewY;
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

// Body: [](const char *name, double value)->void
template<class Body>
static inline void EnumerateAnimationNVP(FBModel *src, const Body& body)
{
    FBAnimationNode *anode = src->AnimationNode;
    if (anode) {
        Each(anode->Nodes, [&body](FBAnimationNode *n) {
            const char *name = n->Name;
            if (name) {
                double value;
                n->ReadData(&value);
                body(name, value);
            }
        });
    }
}


template<class T>
inline std::shared_ptr<T> msmbDevice::createEntity(NodeRecord& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = n.path;
    dst.index = n.index;
    n.dst = ret;
    return ret;
}

ms::TransformPtr msmbDevice::exporttTransform(NodeRecord& n)
{
    auto ret = createEntity<ms::Transform>(n);
    auto& dst = *ret;
    auto src = n.src;

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);

    m_entity_manager.add(ret);
    return ret;
}

ms::CameraPtr msmbDevice::exportCamera(NodeRecord& n)
{
    auto ret = createEntity<ms::Camera>(n);
    auto& dst = *ret;
    auto src = static_cast<FBCamera*>(n.src);

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
    dst.rotation *= mu::rotateY(90.0f * mu::Deg2Rad);

    ExtractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov,
        dst.horizontal_aperture, dst.vertical_aperture, dst.focal_length, dst.focus_distance);

    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msmbDevice::exportLight(NodeRecord& n)
{
    auto ret = createEntity<ms::Light>(n);
    auto& dst = *ret;
    auto src = static_cast<FBLight*>(n.src);

    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
    dst.rotation *= mu::rotateX(90.0f * mu::Deg2Rad);

    ExtractLightData(src, dst.light_type, dst.color, dst.intensity, dst.spot_angle);

    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msmbDevice::exportMeshSimple(NodeRecord& n)
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
            EnumerateAnimationNVP(src, [&weight_table](const char *name, double value) {
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

ms::MeshPtr msmbDevice::exportMesh(NodeRecord& n)
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

void msmbDevice::doExtractMesh(ms::Mesh& dst, FBModel * src)
{
    FBModelVertexData *vd = src->ModelVertexData;
    int num_vertices = vd->GetVertexCount();

    // points
    {
        auto points = (const FBVertex*)vd->GetVertexArray(kFBGeometryArrayID_Point, bake_deformars);
        if (!points)
            return;

        dst.points.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi)
            dst.points[vi] = to_float3(points[vi]);
    }

    // normals
    {
        auto normals = (const FBNormal*)vd->GetVertexArray(kFBGeometryArrayID_Normal, bake_deformars);
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
    if (auto colors_ = vd->GetVertexArray(kFBGeometryArrayID_Color, bake_deformars)) {
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

    if (!bake_deformars) {
        // skin cluster
        if (FBCluster *cluster = src->Cluster) {
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
                    mu::quatf q = mu::invert(mu::rotateXYZ(to_float3(r) * mu::Deg2Rad));
                    bd->bindpose = mu::transform(to_float3(t), q, to_float3(s));
                }

                // weights
                bd->weights.resize_zeroclear(num_vertices);
                for (int vi = 0; vi < n; ++vi) {
                    int i = cluster->VertexGetNumber(vi);
                    float w = (float)cluster->VertexGetWeight(vi);
                    bd->weights[i] = w;
                }
            }
        }

        // blendshapes
        if (FBGeometry *geom = src->Geometry) {
            int num_shapes = geom->ShapeGetCount();
            if (num_shapes) {
                std::map<std::string, float> weight_table;
                EnumerateAnimationNVP(src, [&weight_table](const char *name, double value) {
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
        int offset = vd->GetSubPatchIndexOffset(spi);
        int count = vd->GetSubPatchIndexSize(spi);
        int mid = m_material_records[vd->GetSubPatchMaterial(spi)].id;
        auto idx_begin = indices + offset;
        auto idx_end = idx_begin + count;

        int ngon = 1;
        switch (vd->GetSubPatchPrimitiveType(spi)) {
        case kFBGeometry_POINTS:    ngon = 1; break;
        case kFBGeometry_LINES:     ngon = 2; break;
        case kFBGeometry_TRIANGLES: ngon = 3; break;
        case kFBGeometry_QUADS:     ngon = 4; break;
        // todo: support other topologies (triangle strip, etc)
        default: continue;
        }
        int prim_count = count / ngon;

        dst.indices.insert(dst.indices.end(), idx_begin, idx_end);
        dst.counts.resize(dst.counts.size() + prim_count, ngon);
        dst.material_ids.resize(dst.material_ids.size() + prim_count, mid);
    }

    dst.refine_settings.flags.swap_faces = 1;
    dst.refine_settings.flags.make_double_sided = make_double_sided;
    dst.setupFlags();
}


int msmbDevice::exportTexture(FBTexture* src, FBMaterialTextureType type)
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

bool msmbDevice::exportMaterial(FBMaterial* src, int index)
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

    if (sync_textures && m_dirty_textures) {
        stdmat.setColorMap(exportTexture(src->GetTexture(kFBMaterialTextureDiffuse), kFBMaterialTextureDiffuse));
        stdmat.setEmissionMap(exportTexture(src->GetTexture(kFBMaterialTextureEmissive), kFBMaterialTextureEmissive));
        stdmat.setBumpMap(exportTexture(src->GetTexture(kFBMaterialTextureNormalMap), kFBMaterialTextureNormalMap));
    }
    m_material_manager.add(dst);
    return true;
}

bool msmbDevice::exportMaterials()
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


bool msmbDevice::sendAnimations()
{
    // wait for previous request to complete
    m_sender.wait();

    if (exportAnimations()) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

bool msmbDevice::exportAnimations()
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
    double interval = 1.0 / std::max(animation_sps, 0.01f);

    int reserve_size = int((time_end - time_begin) / interval) + 1;
    for (auto& kvp : m_anim_records) {
        kvp.second.dst->reserve(reserve_size);
    }

    // advance frame and record
    for (double t = time_begin;;) {
        FBTime fbt;
        fbt.SetSecondDouble(t);
        control.Goto(fbt);
        m_anim_time = (float)(t - time_begin) * animation_time_scale;
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

    if (keyframe_reduction) {
        // keyframe reduction
        for (auto& clip : m_animations)
            clip->reduction();

        // erase empty clip
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
            m_animations.end());
    }

    return !m_animations.empty();
}

bool msmbDevice::exportAnimation(FBModel *src, bool force)
{
    if (!src || m_anim_records.find(src) != m_anim_records.end())
        return 0;

    ms::AnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (IsCamera(src)) { // camera
        exportAnimation(src->Parent, true);
        dst = ms::CameraAnimation::create();
        extractor = &msmbDevice::extractCameraAnimation;
    }
    else if (IsLight(src)) { // light
        exportAnimation(src->Parent, true);
        dst = ms::LightAnimation::create();
        extractor = &msmbDevice::extractLightAnimation;
    }
    else if (IsMesh(src)) { // mesh
        EachBones(src, [this](FBModel *bone) {
            exportAnimation(bone, true);
        });
        exportAnimation(src->Parent, true);
        dst = ms::MeshAnimation::create();
        extractor = &msmbDevice::extractMeshAnimation;
    }
    else if (force) { // other
        exportAnimation(src->Parent, true);
        dst = ms::TransformAnimation::create();
        extractor = &msmbDevice::extractTransformAnimation;
    }

    if (dst) {
        auto& rec = m_anim_records[src];
        rec.src = src;
        rec.dst = dst.get();
        rec.extractor = extractor;
        m_animations.front()->animations.push_back(dst);
        return true;
    }
    else {
        return false;
    }
}

void msmbDevice::extractTransformAnimation(ms::Animation& dst_, FBModel* src)
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
    //dst.visible.push_back({ t, vis });

    dst.path = GetPath(src);
}

void msmbDevice::extractCameraAnimation(ms::Animation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::CameraAnimation&>(dst_);
    {
        auto& last = dst.rotation.back();
        last.value *= mu::rotateY(90.0f * mu::Deg2Rad);
    }

    bool ortho;
    float near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance;
    ExtractCameraData(static_cast<FBCamera*>(src), ortho, near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance);

    float t = m_anim_time;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });
}

void msmbDevice::extractLightAnimation(ms::Animation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::LightAnimation&>(dst_);
    {
        auto& last = dst.rotation.back();
        last.value *= mu::rotateX(90.0f * mu::Deg2Rad);
    }

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

void msmbDevice::extractMeshAnimation(ms::Animation & dst_, FBModel * src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::MeshAnimation&>(dst_);

    // blendshape animations
    if (FBGeometry *geom = src->Geometry) {
        int num_shapes = geom->ShapeGetCount();
        if (num_shapes) {
            if (dst.blendshapes.empty()) {
                EnumerateAnimationNVP(src, [&dst](const char *name, double value) {
                    auto bsa = ms::BlendshapeAnimation::create();
                    bsa->name = name;
                    dst.blendshapes.push_back(bsa);
                });
            }

            float t = m_anim_time;
            int idx = 0;
            EnumerateAnimationNVP(src, [&dst, &idx, t](const char *name, double value) {
                dst.blendshapes[idx++]->weight.push_back({ t, (float)value });
            });
        }
    }
}

void msmbDevice::AnimationRecord::operator()(msmbDevice *_this)
{
    (_this->*extractor)(*dst, src);
}
