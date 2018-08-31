#include "pch.h"
#include "msmbDevice.h"
#include "msmbUtils.h"


FBDeviceImplementation(msmbDevice);
FBRegisterDevice("msmbDevice", msmbDevice, "UnityMeshSync", "UnityMeshSync for MotionBuilder", FB_DEFAULT_SDK_ICON);

bool msmbDevice::FBCreate()
{
    FBSystem().Scene->OnChange.Add(this, (FBCallback)&msmbDevice::onSceneChange);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&msmbDevice::onRenderUpdate);
    return true;
}

void msmbDevice::FBDestroy()
{
    FBSystem().Scene->OnChange.Remove(this, (FBCallback)&msmbDevice::onSceneChange);
    FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&msmbDevice::onRenderUpdate);
}

bool msmbDevice::DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo * pEvaluateInfo)
{
    update();
    return true;
}

void msmbDevice::onSceneChange(HIRegister pCaller, HKEventBase pEvent)
{
    FBEventSceneChange SceneChangeEvent = pEvent;
    switch (SceneChangeEvent.Type)
    {
    case kFBSceneChangeSelect:
    case kFBSceneChangeUnselect:
    case kFBSceneChangeReSelect:
    case kFBSceneChangeFocus:
    case kFBSceneChangeSoftSelect:
    case kFBSceneChangeSoftUnselect:
    case kFBSceneChangeHardSelect:
    case kFBSceneChangeTransactionBegin:
    case kFBSceneChangeTransactionEnd:
        return;
    case kFBSceneChangeLoadBegin:
        DeviceOperation(FBDevice::kOpStop);
        return;
    default:
        m_dirty = true;
        break;
    }
}

void msmbDevice::onRenderUpdate(HIRegister pCaller, HKEventBase pEvent)
{
}

void msmbDevice::update()
{
    if (!m_dirty)
        return;
    m_dirty = false;
    kickAsyncSend();
}

bool msmbDevice::isSending() const
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}

void msmbDevice::waitAsyncSend()
{
    if (m_future_send.valid()) {
        m_future_send.wait_for(std::chrono::milliseconds(timeout_ms));
    }
}

void msmbDevice::kickAsyncSend()
{
    // process parallel extract tasks
    if (!m_extract_tasks.empty()) {
        mu::parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](ModelRecords::value_type& kvp) {
            kvp.second();
        });
        m_extract_tasks.clear();
    }


    // begin async send
    m_future_send = std::async(std::launch::async, [this]() {
        ms::Client client(client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = scale_factor;

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

            client.send(del);
            m_deleted.clear();
        }

        // send scene data
        {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.objects = m_objects;
            set.scene.textures = m_textures;
            set.scene.materials = m_materials;
            client.send(set);

            m_objects.clear();
            m_textures.clear();
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


bool msmbDevice::sendScene()
{
    if (isSending()) {
        m_pending = true;
        return false;
    }
    if (exportObject(m_system.Scene->RootModel)) {
        kickAsyncSend();
    }
    return true;
}

int msmbDevice::exportObject(FBModel* src)
{
    int ret = 0;
    if (IsCamera(src)) { // camera
        if (sync_cameras) {
            auto obj = ms::Camera::create();
            m_objects.push_back(obj);
            extractCamera(*obj, static_cast<FBCamera*>(src));
            ++ret;
        }
    }
    else if (IsLight(src)) { // light
        if (sync_lights) {
            auto obj = ms::Light::create();
            m_objects.push_back(obj);
            extractLight(*obj, static_cast<FBLight*>(src));
            ++ret;
        }
    }
    else if (IsBone(src)) { // bone
        if (sync_bones) {
            auto obj = ms::Transform::create();
            m_objects.push_back(obj);
            extractTransform(*obj, src);
            ++ret;
        }
    }
    else if (IsMesh(src)) { // mesh
        if (sync_meshes) {
            auto obj = ms::Mesh::create();
            m_meshes.push_back(obj);
            extractMesh(*obj, src);
            ++ret;
        }
    }

    int num_children = src->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        ret += exportObject(src->Children[i]);

    return ret;
}


static void ExtractTransformData(FBModel* src, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    FBMatrix tmp;
    src->GetMatrix(tmp, kModelTransformation, false, nullptr);

    auto trs = to_float4x4(tmp);
    pos = extract_position(trs);
    rot = extract_rotation(trs);
    scale = extract_scale(trs);
    vis = src->Visibility;
}

void msmbDevice::extractTransform(ms::Transform& dst, FBModel* src)
{
    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
}

void msmbDevice::extractCamera(ms::Camera& dst, FBCamera* src)
{
    extractTransform(dst, src);
    // todo
}

void msmbDevice::extractLight(ms::Light& dst, FBLight* src)
{
    extractTransform(dst, src);
    // todo
}

void msmbDevice::extractMesh(ms::Mesh& dst, FBModel* src)
{
    extractTransform(dst, src);
    doExtractMesh(dst, src);
}

void msmbDevice::doExtractMesh(ms::Mesh & dst, FBModel * src)
{
    auto vd = (FBModelVertexData*)src->ModelVertexData;
    int num_vertices = vd->GetVertexCount();
    auto points = (const FBVertex*)vd->GetVertexArray(kFBGeometryArrayID_Point, false);
    auto normals = (const FBNormal*)vd->GetVertexArray(kFBGeometryArrayID_Normal, false);
    auto uvs = (const FBUV*)vd->GetUVSetArray();

    if (!points)
        return;

    // get vertex arrays
    {
        dst.points.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi)
            dst.points[vi] = to_float3(points[vi]);
    }

    if (normals) {
        dst.normals.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi)
            dst.normals[vi] = to_float3(normals[vi]);
    }
    else {
        dst.refine_settings.flags.gen_normals = 1;
    }

    if (uvs) {
        dst.uv0.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi)
            dst.uv0[vi] = to_float2(uvs[vi]);
        dst.refine_settings.flags.gen_tangents = 1;
    }

    // enumerate subpatches ("submeshes" in Unity's term) and generate indices
    int num_subpatches = vd->GetSubPatchCount();
    auto indices = (const int*)vd->GetIndexArray();
    for (int spi = 0; spi < num_subpatches; ++spi) {
        int offset = vd->GetSubPatchIndexOffset(spi);
        int count = vd->GetSubPatchIndexSize(spi);
        int mid = vd->GetSubPatchMaterialId(spi);
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
        dst.material_ids.resize(dst.counts.size() + prim_count, mid);
    }
}

void msmbDevice::extractTexture(ms::Texture& dst, FBTexture* src)
{
    // todo
}

void msmbDevice::extractMaterial(ms::Material& dst, FBMaterial* src)
{
    // todo
}


bool msmbDevice::sendAnimations()
{
    // wait for previous request to complete
    if (m_future_send.valid()) {
        m_future_send.get();
    }

    if (exportAnimations()) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

int msmbDevice::exportAnimations()
{
    // create default clip
    m_animations.push_back(ms::AnimationClip::create());

    // gather models
    int num_animations = exportAnimation(m_system.Scene->RootModel);
    if (num_animations == 0)
        return 0;


    FBTime time_current = m_system.LocalTime;
    double interval = 1.0 / animation_sps;

    int reserve_size = int(m_time_end / interval) + 1;
    for (auto& kvp : m_anim_tasks) {
        kvp.second.dst->reserve(reserve_size);
    }

    // advance frame and record
    for (double t = 0.0; t < m_time_end; t += interval) {
        FBTime fbt;
        fbt.SetSecondDouble(t);
        m_player_control.Goto(fbt);
        for (auto& kvp : m_anim_tasks)
            kvp.second(this);
    }

    // cleanup
    m_anim_tasks.clear();
    m_player_control.Goto(time_current);

    // keyframe reduction
    for (auto& clip : m_animations)
        clip->reduction();

    // erase empty animation
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
        m_animations.end());

    return num_animations;
}

int msmbDevice::exportAnimation(FBModel * src)
{
    int ret = 0;
    ms::AnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (IsCamera(src)) { // camera
        dst = ms::CameraAnimation::create();
        extractor = &msmbDevice::extractCameraAnimation;
    }
    else if (IsLight(src)) { // light
        dst = ms::LightAnimation::create();
        extractor = &msmbDevice::extractLightAnimation;
    }
    else if (IsBone(src) || IsMesh(src)) { // other
        dst = ms::TransformAnimation::create();
        extractor = &msmbDevice::extractTransformAnimation;
    }

    if (dst) {
        auto& rec = m_anim_tasks[src];
        rec.src = src;
        rec.dst = dst.get();
        rec.extractor = extractor;
        m_animations.front()->animations.push_back(dst);
        ret += 1;
    }

    int num_children = src->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        ret += exportAnimation(src->Children[i]);
    return ret;
}

void msmbDevice::extractTransformAnimation(ms::Animation& dst_, FBModel* src)
{
    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    bool vis = true;
    ExtractTransformData(src, pos, rot, scale, vis);

    float t = m_anim_time * animation_timescale;
    auto& dst = (ms::TransformAnimation&)dst_;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    //dst.visible.push_back({ t, vis });
}

void msmbDevice::extractCameraAnimation(ms::Animation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::CameraAnimation&>(dst_);
    // todo
}

void msmbDevice::extractLightAnimation(ms::Animation& dst_, FBModel* src)
{
    extractTransformAnimation(dst_, src);

    auto& dst = static_cast<ms::LightAnimation&>(dst_);
    // todo
}

void msmbDevice::AnimationRecord::operator()(msmbDevice *_this)
{
    (_this->*extractor)(*dst, src);
}
