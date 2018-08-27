#include "pch.h"
#include "MeshSyncClientMotionBuilder.h"


FBLibraryDeclare(msmbDevice)
{
    FBLibraryRegister(msmbDevice);
    FBLibraryRegister(msmbLayout);
}
FBLibraryDeclareEnd;

bool FBLibrary::LibInit() { return true; }
bool FBLibrary::LibOpen() { return true; }
bool FBLibrary::LibReady() { return true; }
bool FBLibrary::LibClose() { return true; }
bool FBLibrary::LibRelease() { return true; }



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



FBDeviceLayoutImplementation(msmbLayout);
FBRegisterDeviceLayout(msmbLayout, "UnityMeshSync", FB_DEFAULT_SDK_ICON);

bool msmbLayout::FBCreate()
{
    m_device = (msmbDevice*)(FBDevice*)Device;
    return true;
}

void msmbLayout::FBDestroy()
{
}
