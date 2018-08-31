#pragma once

class msmbDevice : public FBDevice
{
    FBDeviceDeclare(msmbDevice, FBDevice);
public:
    bool FBCreate() override;
    void FBDestroy() override;

    bool DeviceOperation(kDeviceOperations pOperation) override;
    void DeviceTransportNotify(kTransportMode pMode, FBTime pTime, FBTime pSystem) override;
    bool DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo) override;

    bool sendScene();
    bool sendAnimations();

private:
    void onSceneChange(HIRegister pCaller, HKEventBase pEvent);
    void onRenderUpdate(HIRegister pCaller, HKEventBase pEvent);

    void update();
    bool isSending() const;
    void waitAsyncSend();
    void kickAsyncSend();

    bool exportObject(FBModel* src, bool force);
    void extractTransform(ms::Transform& dst, FBModel* src);
    void extractCamera(ms::Camera& dst, FBCamera* src);
    void extractLight(ms::Light& dst, FBLight* src);
    void extractTexture(ms::Texture& dst, FBTexture* src);
    void extractMaterial(ms::Material& dst, FBMaterial* src);
    void extractMesh(ms::Mesh& dst, FBModel* src);
    void doExtractMesh(ms::Mesh& dst, FBModel* src);

    bool exportAnimations();
    bool exportAnimation(FBModel* src, bool force);
    void extractTransformAnimation(ms::Animation& dst, FBModel* src);
    void extractCameraAnimation(ms::Animation& dst, FBModel* src);
    void extractLightAnimation(ms::Animation& dst, FBModel* src);


    using NodeRecords = std::map<FBModel*, ms::TransformPtr>;
    using ExtractTasks = std::vector<std::function<void()>>;

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msmbDevice::*)(ms::Animation& dst, FBModel *src);

        ms::Animation *dst = nullptr;
        FBModel *src = nullptr;
        extractor_t extractor = nullptr;

        void operator()(msmbDevice *_this);
    };
    using AnimationRecords = std::map<FBModel*, AnimationRecord>;

private:
    bool m_dirty = true;
    bool m_pending = false;
    float m_anim_time = 0.0f;

    NodeRecords m_node_records;
    ExtractTasks m_extract_tasks;
    AnimationRecords m_anim_records;

    std::vector<ms::TransformPtr>       m_objects;
    std::vector<ms::MeshPtr>            m_meshes;
    std::vector<ms::TexturePtr>         m_textures;
    std::vector<ms::MaterialPtr>        m_materials;
    std::vector<ms::AnimationClipPtr>   m_animations;
    std::vector<ms::ConstraintPtr>      m_constraints;
    std::vector<std::string>            m_deleted;
    std::future<void>                   m_future_send;

public:
    ms::ClientSettings client_settings;
    int  timeout_ms = 5000;
    float scale_factor = 100.0f;
    float time_scale = 1.0f;
    float samples_per_second = 3.0f;

    bool auto_sync = false;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_bones = true;
    bool sync_meshes = true;
    bool sync_textures = false;
    bool sync_material = false;
};
