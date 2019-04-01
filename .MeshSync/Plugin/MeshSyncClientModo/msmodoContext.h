#pragma once

#include "msmodoInterface.h"

namespace ms {

    template<>
    class IDGenerator<CLxUser_Item> : public IDGenerator<void*>
    {
    public:
        int getID(const CLxUser_Item& v)
        {
            return getIDImpl((void*&)v.m_loc);
        }
    };

} // namespace ms

struct LxItemKey
{
    void *key;

    LxItemKey() : key(nullptr) {}
    LxItemKey(const CLxUser_Item& v) : key((void*&)v.m_loc) {}
    bool operator<(const LxItemKey& v) const { return key < v.key; }
    bool operator==(const LxItemKey& v) const { return key == v.key; }
    bool operator!=(const LxItemKey& v) const { return key != v.key; }
};

class msmodoContext;



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
    bool sync_mesh_instances = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_constraints = false;
    bool reduce_keyframes = true;

    // import settings
    bool bake_skin = false;
    bool bake_cloth = false;
};


class msmodoContext : private msmodoInterface
{
using super = msmodoInterface;
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
    struct TreeNode : public mu::noncopyable
    {
        CLxUser_Item item;

        std::string name;
        std::string path;
        int id = ms::InvalidID;
        int index = 0;

        ms::TransformPtr dst_obj;
        ms::AnimationPtr dst_anim;
        RawVector<const char*> material_names; // per-face material names.

        using AnimationExtractor = void (msmodoContext::*)(TreeNode& node);
        AnimationExtractor anim_extractor = nullptr;

        void clearState();
        void doExtractAnimation(msmodoContext *self);
        void resolveMaterialIDs(const std::vector<ms::MaterialPtr>& materials);
    };

    void exportMaterials();
    ms::MaterialPtr exportMaterial(CLxUser_Item obj);

    ms::TransformPtr exportObject(CLxUser_Item obj);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode& node);
    ms::TransformPtr exportMeshInstance(TreeNode& node);
    ms::CameraPtr exportCamera(TreeNode& node);
    ms::LightPtr exportLight(TreeNode& node);
    ms::MeshPtr exportMesh(TreeNode& node);

    int exportAnimations(SendScope scope);
    bool exportAnimation(CLxUser_Item obj);
    void extractTransformAnimationData(TreeNode& node);
    void extractCameraAnimationData(TreeNode& node);
    void extractLightAnimationData(TreeNode& node);
    void extractMeshAnimationData(TreeNode& node);

    void kickAsyncSend();

    void extractTransformData(TreeNode& n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis);
    void extractCameraData(TreeNode& n, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance);
    void extractLightData(TreeNode& n, ms::Light::LightType& type, mu::float4& color, float& intensity, float& spot_angle);

private:
    msmodoSettings m_settings;
    ms::IDGenerator<CLxUser_Item> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;

    int m_material_index_seed = 0;
    int m_entity_index_seed = 0;
    std::vector<ms::MaterialPtr> m_materials; // sorted by name
    std::map<LxItemKey, TreeNode> m_tree_nodes;
    std::vector<TreeNode*> m_anim_nodes;
    std::vector<ms::AnimationClipPtr> m_animations;
    SendScope m_pending_scope = SendScope::None;
    bool m_ignore_update = false;
    float m_anim_time = 0.0f;
};

#define msmodoGetInstance() msmodoContext::getInstance()
#define msmodoGetSettings() msmodoGetInstance().getSettings()
