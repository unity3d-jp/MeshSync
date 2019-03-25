#pragma once

struct msmodoSettings
{
    ms::ClientSettings client_settings;

    float scale_factor = 1.0f;
    float animation_time_scale = 1.0f;
    float animation_sps = 2.0f;
    int  timeout_ms = 5000;
    bool auto_sync = false;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool make_double_sided = false;
    bool bake_deformers = false;
    bool sync_blendshapes = true;
    bool sync_bones = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_constraints = false;
    bool remove_namespace = true;
    bool reduce_keyframes = true;

    // import settings
    bool bake_skin = false;
    bool bake_cloth = false;
};


class msmodoContext
{
public:
    enum class SendScope
    {
        None,
        All,
        Updated,
        Selected,
    };


    static msmodoContext& getInstance();

    msmodoContext();
    ~msmodoContext();

    msmodoSettings& getSettings();

    void update();
    bool sendScene(SendScope scope, bool dirty_all);
    bool sendAnimations(SendScope scope);

    bool recvScene();

private:
    ms::TransformPtr exportObject(CLxLoc_Item& obj, bool force);
    ms::TransformPtr exportTransform(CLxLoc_Item& obj);
    ms::CameraPtr exportCamera(CLxLoc_Item& obj);
    ms::LightPtr exportLight(CLxLoc_Item& obj);
    ms::MeshPtr exportMesh(CLxLoc_Item& obj);

    int exportAnimations(SendScope scope);
    bool exportAnimation(CLxLoc_Item& obj, bool force);
    void extractTransformAnimationData(ms::Animation& dst, CLxLoc_Item& obj);
    void extractCameraAnimationData(ms::Animation& dst, CLxLoc_Item& obj);
    void extractLightAnimationData(ms::Animation& dst, CLxLoc_Item& obj);
    void extractMeshAnimationData(ms::Animation& dst, CLxLoc_Item& obj);

    void kickAsyncSend();

private:
    msmodoSettings m_settings;

    std::vector<ms::AnimationClipPtr> m_animations;

    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
};

#define msmodoGetInstance() msmodoContext::getInstance()
#define msmodoGetSettings() msmodoGetInstance().getSettings()
