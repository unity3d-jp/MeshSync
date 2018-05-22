#pragma once

#include <string>
#include <vector>
#include <memory>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msFoundation.h"

#ifdef GetMessage
    #undef GetMessage
#endif

namespace ms {


class Entity : public std::enable_shared_from_this<Entity>
{
public:
    enum class Type
    {
        Unknown,
        Transform,
        Camera,
        Light,
        Mesh,
    };

    int id = 0;
    std::string path;

    static Entity* make(std::istream& is);

    virtual ~Entity();
    virtual Type getType() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();

    const char* getName() const; // get name (leaf) from path
};
HasSerializer(Entity);
using EntityPtr = std::shared_ptr<Entity>;



class Transform : public Entity
{
using super = Entity;
public:
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   scale = float3::one();
    int index = 0;

    bool visible = true;
    bool visible_hierarchy = true;
    std::string reference;


    static Transform* make(std::istream& is);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    float4x4 toMatrix() const;
    void assignMatrix(const float4x4& v);
    void applyMatrix(const float4x4& v);

    virtual void convertHandedness(bool x, bool yz);
    virtual void applyScaleFactor(float scale);
};
HasSerializer(Transform);
using TransformPtr = std::shared_ptr<Transform>;


class Camera : public Transform
{
using super = Transform;
public:
    bool is_ortho = false;
    float fov = 30.0f;
    float near_plane = 0.3f;
    float far_plane = 1000.0f;

    // for physical camera
    float vertical_aperture = 0.0f;
    float horizontal_aperture = 0.0f;
    float focal_length = 0.0f;
    float focus_distance = 0.0f;

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    void applyScaleFactor(float scale) override;
};
HasSerializer(Camera);
using CameraPtr = std::shared_ptr<Camera>;



class Light : public Transform
{
using super = Transform;
public:
    enum class LightType
    {
        Spot,
        Directional,
        Point,
        Area,
    };

    LightType light_type = LightType::Directional;
    float4 color = float4::one();
    float intensity = 1.0f;
    float range = 0.0f;
    float spot_angle = 30.0f; // for spot light

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    void applyScaleFactor(float scale) override;
};
HasSerializer(Light);
using LightPtr = std::shared_ptr<Light>;



// Mesh

struct MeshDataFlags
{
    uint32_t has_refine_settings : 1;
    uint32_t has_indices : 1;
    uint32_t has_counts : 1;
    uint32_t has_points : 1;
    uint32_t has_normals : 1;
    uint32_t has_tangents : 1;
    uint32_t has_uv0 : 1;
    uint32_t has_uv1 : 1;
    uint32_t has_colors : 1;
    uint32_t has_material_ids : 1;
    uint32_t has_bones : 1;
    uint32_t has_blendshapes : 1;
    uint32_t apply_trs : 1;
};

struct MeshRefineFlags
{
    uint32_t split : 1;
    uint32_t no_reindexing : 1;
    uint32_t triangulate : 1;
    uint32_t optimize_topology : 1;
    uint32_t swap_handedness : 1;
    uint32_t swap_yz : 1;
    uint32_t swap_faces : 1;
    uint32_t gen_normals : 1;
    uint32_t gen_normals_with_smooth_angle : 1;
    uint32_t flip_normals : 1; // 10
    uint32_t gen_tangents : 1;
    uint32_t apply_local2world : 1;
    uint32_t apply_world2local : 1;
    uint32_t bake_skin : 1;
    uint32_t bake_cloth : 1;

    uint32_t invert_v : 1;
    uint32_t mirror_x : 1;
    uint32_t mirror_y : 1;
    uint32_t mirror_z : 1;
    uint32_t mirror_x_weld : 1; // 20
    uint32_t mirror_y_weld : 1;
    uint32_t mirror_z_weld : 1;
};

struct MeshRefineSettings
{
    MeshRefineFlags flags = { 0 };
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f;
    uint32_t split_unit = 65000;
    uint32_t max_bones_par_vertices = 4;
    float4x4 local2world = float4x4::identity();
    float4x4 world2local = float4x4::identity();
};

struct SubmeshData
{
    enum class Topology
    {
        Points,
        Lines,
        Triangles,
        Quads,
    };

    IArray<int> indices;
    Topology topology = Topology::Triangles;
    int material_id = 0;
};

struct SplitData
{
    int index_count = 0;
    int index_offset = 0;
    int vertex_count = 0;
    int vertex_offset = 0;
    IArray<SubmeshData> submeshes;
    float3 bound_center = float3::zero();
    float3 bound_size = float3::zero();
};

struct BlendShapeData
{
    struct Frame
    {
        float weight = 0.0f;
        RawVector<float3> points;
        RawVector<float3> normals;
        RawVector<float3> tangents;

        uint32_t getSerializeSize() const;
        void serialize(std::ostream& os) const;
        void deserialize(std::istream& is);
        void clear();
    };

    std::string name;
    float weight = 0.0f;
    std::vector<Frame> frames;

    static BlendShapeData* make(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
HasSerializer(BlendShapeData::Frame);
HasSerializer(BlendShapeData);
using BlendShapeDataPtr = std::shared_ptr<BlendShapeData>;

struct BoneData : public std::enable_shared_from_this<BoneData>
{
    std::string path;
    float4x4 bindpose = float4x4::identity();
    RawVector<float> weights;

    static BoneData* make(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
HasSerializer(BoneData);
using BoneDataPtr = std::shared_ptr<BoneData>;

class Mesh : public Transform
{
using super = Transform;
public:
    MeshDataFlags     flags = { 0 };
    MeshRefineSettings refine_settings;

    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv0, uv1;
    RawVector<float4> colors;
    RawVector<int>    counts;
    RawVector<int>    indices;
    RawVector<int>    material_ids;

    std::string root_bone;
    std::vector<BoneDataPtr> bones;
    std::vector<BlendShapeDataPtr> blendshapes;

    // non-serialized
    RawVector<Weights4> weights4;
    RawVector<float3> tmp_normals;
    RawVector<float2> tmp_uv0, tmp_uv1;
    RawVector<float4> tmp_colors;
    RawVector<int> remap_normals, remap_uv0, remap_uv1, remap_colors;

    RawVector<Weights4> tmp_weights4;
    std::vector<SubmeshData> submeshes;
    std::vector<SplitData> splits;

public:
    Mesh();
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    void convertHandedness(bool x, bool yz) override;
    void applyScaleFactor(float scale) override;

    void refine(const MeshRefineSettings& mrs);
    void applyMirror(const float3& plane_n, float plane_d, bool welding = false);
    void applyTransform(const float4x4& t);

    void generateWeights4();
    void setupFlags();

    void convertHandedness_Mesh(bool x, bool yz);
    void convertHandedness_BlendShapes(bool x, bool yz);
    void convertHandedness_Bones(bool x, bool yz);

    BoneDataPtr addBone(const std::string& path);
    BlendShapeDataPtr addBlendShape(const std::string& name);
};
HasSerializer(Mesh);
using MeshPtr = std::shared_ptr<Mesh>;


class Constraint;
using ConstraintPtr = std::shared_ptr<Constraint>;

class Animation;
using AnimationPtr = std::shared_ptr<Animation>;

class AnimationClip;
using AnimationClipPtr = std::shared_ptr<AnimationClip>;

class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class Material;
using MaterialPtr = std::shared_ptr<Material>;


enum class Handedness
{
    Left,
    Right,
    LeftZUp,
    RightZUp,
};

struct SceneSettings
{
    std::string name = "Untitled";
    Handedness handedness = Handedness::Left;
    float scale_factor = 1.0f;

    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
HasSerializer(SceneSettings);

struct Scene : public std::enable_shared_from_this<Scene>
{
public:
    SceneSettings settings;
    std::vector<TransformPtr> objects;
    std::vector<ConstraintPtr> constraints;
    std::vector<AnimationClipPtr> animations;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;

public:
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
};
HasSerializer(Scene);
using ScenePtr = std::shared_ptr<Scene>;


} // namespace ms
