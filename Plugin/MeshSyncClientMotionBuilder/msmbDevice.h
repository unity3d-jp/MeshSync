#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

class msmbDevice : public FBDevice
{
FBDeviceDeclare(msmbDevice, FBDevice);
public:
    bool FBCreate() override;
    void FBDestroy() override;

    bool DeviceOperation(kDeviceOperations pOperation) override;
    void DeviceTransportNotify(kTransportMode pMode, FBTime pTime, FBTime pSystem) override;

    bool sendScene(bool force_all);
    bool sendAnimations();

private:
    struct NodeRecord
    {
        std::string name;
        std::string path;
        int id = ms::InvalidID;
        int index = 0;
        FBModel *src = nullptr;
        ms::TransformPtr dst;
        bool exist = false;

        ms::Identifier getIdentifier() const;
    };
    using NodeRecords = std::map<FBModel*, NodeRecord>;
    using ExtractTasks = std::vector<std::function<void()>>;

    struct TextureRecord : public mu::noncopyable
    {
        int id = ms::InvalidID;
        ms::Texture *dst = nullptr;
    };
    using TextureRecords = std::map<FBTexture*, TextureRecord>;

    struct MaterialRecord : public mu::noncopyable
    {
        int id = ms::InvalidID;
        ms::Material *dst = nullptr;
    };
    using MaterialRecords = std::map<FBMaterial*, MaterialRecord>;

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
    void onSceneChange(HIRegister pCaller, HKEventBase pEvent);
    void onDataUpdate(HIRegister pCaller, HKEventBase pEvent);
    void onRender(HIRegister pCaller, HKEventBase pEvent);
    void onSynchronization(HIRegister pCaller, HKEventBase pEvent);

    void update();
    void kickAsyncSend();

    ms::TransformPtr exportObject(FBModel* src, bool force);
    template<class T> std::shared_ptr<T> createEntity(NodeRecord& n);
    ms::TransformPtr exporttTransform(NodeRecord& n);
    ms::CameraPtr exportCamera(NodeRecord& n);
    ms::LightPtr exportLight(NodeRecord& n);
    ms::MeshPtr exportMeshSimple(NodeRecord& n);
    ms::MeshPtr exportMesh(NodeRecord& n);
    void doExtractMesh(ms::Mesh& dst, FBModel* src);

    int exportTexture(FBTexture* src, FBMaterialTextureType type);
    bool exportMaterial(FBMaterial* src, int index);
    bool exportMaterials();

    bool exportAnimations();
    bool exportAnimation(FBModel* src, bool force);
    void extractTransformAnimation(ms::Animation& dst, FBModel* src);
    void extractCameraAnimation(ms::Animation& dst, FBModel* src);
    void extractLightAnimation(ms::Animation& dst, FBModel* src);
    void extractMeshAnimation(ms::Animation& dst, FBModel* src);

private:
    bool m_data_updated = false;
    bool m_dirty_meshes = true;
    bool m_dirty_textures = true;
    bool m_pending = false;

    float m_anim_time = 0.0f;
    int m_texture_id_seed = 0;
    int m_node_index_seed = 0;

    NodeRecords m_node_records;
    TextureRecords m_texture_records;
    MaterialRecords m_material_records;
    ExtractTasks m_extract_tasks;
    AnimationRecords m_anim_records;

    std::vector<ms::AnimationClipPtr> m_animations;

    ms::IDGenerator<FBMaterial*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;

public:
    ms::ClientSettings client_settings;
    int  timeout_ms = 5000;
    float scale_factor = 100.0f;
    float animation_time_scale = 1.0f;
    float animation_sps = 3.0f;

    bool auto_sync = false;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_bones = true;
    bool sync_meshes = true;
    bool sync_textures = true;
    bool make_double_sided = false;
    bool bake_deformars = false;
    bool parallel_extraction = true;
};
