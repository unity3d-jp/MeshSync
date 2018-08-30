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

public:
    ms::ClientSettings client_settings;
    float scale_factor = 1.0f;
    float animation_timescale = 1.0f;
    float animation_sps = 3.0f;

    bool auto_sync = false;
    bool sync_cameras = false;
    bool sync_lights = false;
    bool sync_meshes = false;
    bool sync_textures = false;
    bool sync_material = false;
};
