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
    send();
}

void msmbDevice::send()
{
    // todo:
}


void msmbDevice::extractScene()
{
    m_system.Scene;
}

void msmbDevice::extract(ms::Scene& dst, FBModel* src)
{
    if (src->Is(FBCamera::TypeInfo)) { // camera
        if (sync_cameras) {
            auto obj = ms::Camera::create();
            dst.objects.push_back(obj);
            extractCamera(*obj, static_cast<FBCamera*>(src));
        }
    }
    else if (src->Is(FBLight::TypeInfo)) { // light
        if (sync_lights) {
            auto obj = ms::Light::create();
            dst.objects.push_back(obj);
            extractLight(*obj, static_cast<FBLight*>(src));
        }
    }
    else if (src->Is(FBModelOptical::TypeInfo)) { // optional model
        // ignore
    }
    else {
        if (sync_meshes) {
            auto obj = ms::Mesh::create();
            dst.objects.push_back(obj);
            extractMesh(*obj, src);
        }
        else {
            auto obj = ms::Transform::create();
            dst.objects.push_back(obj);
            extractTransform(*obj, src);
        }
    }

    int num_children = src->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        extract(dst, src->Children[i]);
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
    // todo
}

void msmbDevice::extractTexture(ms::Texture& dst, FBTexture* src)
{

}


void msmbDevice::extractMaterial(ms::Material& dst, FBMaterial* src)
{
}


void msmbDevice::extractAnimations()
{
    // create default clip
    m_animations.push_back(ms::AnimationClip::create());

    // gather models
    extractAnimation(m_system.Scene->RootModel);


    float time_step = 1.0f / animation_sps;
    float time_end = m_time_end; // 
    for (float t = 0.0f; t < time_end; t += time_step) {
        // advance time and record
        FBTime fbt;
        fbt.SetSecondDouble(t);
        m_player_control.Goto(fbt);
        for (auto& kvp : m_anim_tasks)
            kvp.second(this);
    }
    // cleanup
    m_anim_tasks.clear();

    // reduction
    for (auto& clip : m_animations)
        clip->reduction();

    // erase empty animation
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
        m_animations.end());
}

void msmbDevice::extractAnimation(FBModel * src)
{
    ms::AnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (src->Is(FBCamera::TypeInfo)) { // camera
        dst = ms::CameraAnimation::create();
        extractor = &msmbDevice::extractCameraAnimation;
    }
    else if (src->Is(FBLight::TypeInfo)) { // light
        dst = ms::LightAnimation::create();
        extractor = &msmbDevice::extractLightAnimation;
    }
    else if (src->Is(FBModelOptical::TypeInfo)) { // optional model
        // ignore
    }
    else {
        dst = ms::TransformAnimation::create();
        extractor = &msmbDevice::extractTransformAnimation;
    }

    if (dst) {
        auto& rec = m_anim_tasks[src];
        rec.src = src;
        rec.dst = dst.get();
        rec.extractor = extractor;
        m_animations.front()->animations.push_back(dst);
    }

    int num_children = src->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        extractAnimation(src->Children[i]);
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
