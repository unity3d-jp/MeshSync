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


void msmbDevice::extract(FBModel* src, ms::Scene& dst)
{
    if (src->Is(FBCamera::TypeInfo)) { // camera
        if (m_sync_cameras) {
            auto obj = ms::Camera::create();
            dst.objects.push_back(obj);
            extractCamera(static_cast<FBCamera*>(src), *obj);
        }
    }
    else if (src->Is(FBLight::TypeInfo)) { // light
        if (m_sync_lights) {
            auto obj = ms::Light::create();
            dst.objects.push_back(obj);
            extractLight(static_cast<FBLight*>(src), *obj);
        }
    }
    else if (src->Is(FBModelOptical::TypeInfo)) { // optional model
        // ignore
    }
    else {
        if (m_sync_meshes) {
            auto obj = ms::Mesh::create();
            dst.objects.push_back(obj);
            extractMesh(src, *obj);
        }
        else {
            auto obj = ms::Transform::create();
            dst.objects.push_back(obj);
            extractTransform(src, *obj);
        }
    }

    int num_children = src->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        extract(src->Children[i], dst);
}

void msmbDevice::extractTransform(FBModel* src, ms::Transform& dst)
{
    FBMatrix tmp;
    src->GetMatrix(tmp, kModelTransformation, false, nullptr);

    auto trs = to_float4x4(tmp);
    dst.position = extract_position(trs);
    dst.rotation = extract_rotation(trs);
    dst.scale = extract_scale(trs);
}

void msmbDevice::extractCamera(FBCamera* src, ms::Camera& dst)
{
    extractTransform(src, dst);
    // todo
}

void msmbDevice::extractLight(FBLight* src, ms::Light& dst)
{
    extractTransform(src, dst);
    // todo
}

void msmbDevice::extractMesh(FBModel* src, ms::Mesh& dst)
{
    extractTransform(src, dst);
    // todo
}

void msmbDevice::extractTexture(FBModel* src, ms::Texture& dst)
{

}

void msmbDevice::extractMaterial(FBModel* src, ms::Material& dst)
{

}

void msmbDevice::extractAnimation(FBAnimationNode* src, ms::Animation& dst)
{
    if (auto fcurve = src->FCurve) {
        int num_keys = fcurve->Keys.GetCount();
        for (int ki = 0; ki < num_keys; ki++) {

        }
    }

    int num_nodes = src->Nodes.GetCount();
    for (int ni = 0; ni < num_nodes; ++ni) {
        extractAnimation(src->Nodes[ni], dst);
    }
}
