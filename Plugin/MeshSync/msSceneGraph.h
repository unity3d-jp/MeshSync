#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"

#ifdef GetMessage
    #undef GetMessage
#endif

namespace ms {

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    enum class TypeID
    {
        Unknown,
        Transform,
        Camera,
        Light,
        Mesh,
    };

    int id = 0;
    int index = 0;
    std::string path;

    virtual ~Entity();
    virtual TypeID getTypeID() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();

    const char* getName() const; // get name (leaf) from path
};


// time-value pair
template<class T>
struct TVP
{
    float time;
    T value;
};

class Animation : public std::enable_shared_from_this<Animation>
{
public:
    virtual ~Animation();
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();

    virtual bool empty() const;
};
using AnimationPtr = std::shared_ptr<Animation>;


class Material : public std::enable_shared_from_this<Material>
{
public:
    int id = 0;
    std::string name;
    float4 color = float4::one();

    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
using MaterialPtr = std::shared_ptr<Material>;


class Transform : public Entity
{
using super = Entity;
public:
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   scale = float3::one();

    bool visible = true;
    bool visible_hierarchy = true;
    std::string reference;
    AnimationPtr animation;


    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    float4x4 toMatrix() const;
    void assignMatrix(const float4x4& v);
    void applyMatrix(const float4x4& v);

    virtual void createAnimation();
    virtual void convertHandedness(bool x, bool yz);
    virtual void applyScaleFactor(float scale);

public:
    // for python binding
    void addTranslationKey(float t, const float3& v);
    void addRotationKey(float t, const quatf& v);
    void addScaleKey(float t, const float3& v);
};
using TransformPtr = std::shared_ptr<Transform>;

class TransformAnimation : public Animation
{
public:
    RawVector<TVP<float3>>  translation;
    RawVector<TVP<quatf>>   rotation;
    RawVector<TVP<float3>>  scale;
    RawVector<TVP<bool>>    visible;

    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
};


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

    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    void createAnimation() override;
    void applyScaleFactor(float scale) override;

public:
    // for python binding
    void addFovKey(float t, float v);
    void addNearPlaneKey(float t, float v);
    void addFarPlaneKey(float t, float v);
    void addHorizontalApertureKey(float t, float v);
    void addVerticalApertureKey(float t, float v);
    void addFocalLengthKey(float t, float v);
    void addFocusDistanceKey(float t, float v);
};
using CameraPtr = std::shared_ptr<Camera>;

class CameraAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float>>   fov;
    RawVector<TVP<float>>   near_plane;
    RawVector<TVP<float>>   far_plane;
    RawVector<TVP<float>>   horizontal_aperture;
    RawVector<TVP<float>>   vertical_aperture;
    RawVector<TVP<float>>   focal_length;
    RawVector<TVP<float>>   focus_distance;

    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
};


class Light : public Transform
{
using super = Transform;
public:
    enum class Type
    {
        Spot,
        Directional,
        Point,
        Area,
    };

    Type type = Type::Directional;
    float4 color = float4::one();
    float intensity = 1.0f;
    float range = 0.0f;
    float spot_angle = 30.0f; // for spot light

    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;

    void createAnimation() override;
    void applyScaleFactor(float scale) override;

public:
    // for python binding
    void addColorKey(float t, const float4& v);
    void addIntensityKey(float t, float v);
    void addRangeKey(float t, float v);
    void addSpotAngleKey(float t, float v);
};
using LightPtr = std::shared_ptr<Light>;

class LightAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float4>>  color;
    RawVector<TVP<float>>   intensity;
    RawVector<TVP<float>>   range;
    RawVector<TVP<float>>   spot_angle; // for spot light

    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
};


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

    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
using BlendShapeDataPtr = std::shared_ptr<BlendShapeData>;

struct BoneData : public std::enable_shared_from_this<BoneData>
{
    std::string path;
    float4x4 bindpose = float4x4::identity();
    RawVector<float> weights;

    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
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
    TypeID getTypeID() const override;
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

public:
    // for python binding
    void addVertex(const float3& v);
    void addNormal(const float3& v);
    void addUV(const float2& v);
    void addColor(const float4& v);

    void addCount(int v);
    void addIndex(int v);
    void addMaterialID(int v);

    BoneDataPtr addBone(const std::string& path);
    BlendShapeDataPtr addBlendShape(const std::string& name);
};
using MeshPtr = std::shared_ptr<Mesh>;


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

struct Scene : public std::enable_shared_from_this<Scene>
{
public:
    SceneSettings settings;
    std::vector<MeshPtr>      meshes;
    std::vector<TransformPtr> transforms;
    std::vector<CameraPtr>    cameras;
    std::vector<LightPtr>     lights;
    std::vector<MaterialPtr>  materials;

public:
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
};
using ScenePtr = std::shared_ptr<Scene>;




class Message
{
public:
    enum class Type
    {
        Unknown,
        Get,
        Set,
        Delete,
        Fence,
        Text,
        Screenshot,
    };

    virtual ~Message();
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual bool deserialize(std::istream& is);
};
using MessagePtr = std::shared_ptr<Message>;


struct GetFlags
{
    uint32_t get_transform : 1;
    uint32_t get_points : 1;
    uint32_t get_normals : 1;
    uint32_t get_tangents : 1;
    uint32_t get_uv0 : 1;
    uint32_t get_uv1 : 1;
    uint32_t get_colors : 1;
    uint32_t get_indices : 1;
    uint32_t get_material_ids : 1;
    uint32_t get_bones : 1;
    uint32_t get_blendshapes : 1;
    uint32_t apply_culling : 1;
};


class GetMessage : public Message
{
using super = Message;
public:
    GetFlags flags = {0};
    SceneSettings scene_settings;
    MeshRefineSettings refine_settings;

    // non-serializable
    std::shared_ptr<std::atomic_int> wait_flag;

public:
    GetMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};


class SetMessage : public Message
{
using super = Message;
public:
    Scene scene;

public:
    SetMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};


class DeleteMessage : public Message
{
using super = Message;
public:
    struct Identifier
    {
        std::string path;
        int id;

        uint32_t getSerializeSize() const;
        void serialize(std::ostream& os) const;
        void deserialize(std::istream& is);
    };
    std::vector<Identifier> targets;

    DeleteMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};


class FenceMessage : public Message
{
using super = Message;
public:
    enum class FenceType
    {
        Unknown,
        SceneBegin,
        SceneEnd,
    };

    FenceType type = FenceType::Unknown;

    ~FenceMessage() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};


class TextMessage : public Message
{
using super = Message;
public:
    enum class Type
    {
        Normal,
        Warning,
        Error,
    };

    ~TextMessage() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;

public:
    std::string text;
    Type type = Type::Normal;
};


class ScreenshotMessage : public Message
{
using super = Message;
public:

    // non-serializable
    std::shared_ptr<std::atomic_int> wait_flag;

public:
    ScreenshotMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};

} // namespace ms
