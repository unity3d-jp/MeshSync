#pragma once


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

    void extract(FBModel* src, ms::Scene& dst);
    void extractTransform(FBModel* src, ms::Transform& dst);
    void extractCamera(FBCamera* src, ms::Camera& dst);
    void extractLight(FBLight* src, ms::Light& dst);
    void extractMesh(FBModel* src, ms::Mesh& dst);
    void extractTexture(FBModel* src, ms::Texture& dst);
    void extractMaterial(FBModel* src, ms::Material& dst);
    void extractAnimation(FBAnimationNode* src, ms::Animation& dst);

private:
    bool m_dirty = true;

    bool m_auto_sync = false;
    bool m_sync_cameras = false;
    bool m_sync_lights = false;
    bool m_sync_meshes = false;
    bool m_sync_textures = false;
    bool m_sync_material = false;
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