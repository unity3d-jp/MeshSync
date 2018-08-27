#pragma once

#define MOBULIVELINK__CLASSNAME		FMobuLiveLink
#define MOBULIVELINK__CLASSSTR		"MobuLiveLink"

class msmbDevice : public FBDevice
{
FBDeviceDeclare(msmbDevice, FBDevice);
public:
    bool FBCreate() override;
    void FBDestroy() override;

    bool DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo) override;

private:
    void onSceneChange(HIRegister pCaller, HKEventBase pEvent);
    void onRenderUpdate(HIRegister pCaller, HKEventBase pEvent);

    void update();
    void send();

private:
    bool m_dirty = true;
};


class msmbLayout : public FBDeviceLayout
{
FBDeviceLayoutDeclare(msmbLayout, FBDeviceLayout);
public:
    bool FBCreate() override;
    void FBDestroy() override;

private:
    FBSystem    m_system;
    msmbDevice* m_device;
};