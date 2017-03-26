#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include "MeshUtils/RawVector.h"
#include "MeshUtils/MeshUtils.h"

#ifdef GetMessage
    #undef GetMessage
#endif


namespace ms {
using namespace mu;

void LogImpl(const char *fmt, ...);
#define msLogInfo(...)    ::ms::LogImpl("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::ms::LogImpl("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::ms::LogImpl("MeshSync error: " __VA_ARGS__)

extern const int ProtocolVersion;

class Entity
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

    const char* getName() const; // get name (leaf) from path
};


struct TRS
{
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   rotation_eularZXY = float3::zero();
    float3   scale = float3::one();

    float4x4 toMatrix() const;
};

// time-value pair
template<class T>
struct TVP
{
    float time;
    T value;
};

class Animation
{
public:
    virtual ~Animation();
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);

    virtual bool empty() const;
};
using AnimationPtr = std::shared_ptr<Animation>;

struct Material
{
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
    TRS transform;
    AnimationPtr animation;
    std::string reference;


    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    virtual void createAnimation();
    virtual void swapHandedness();
    virtual void applyScaleFactor(float scale);
};
using TransformPtr = std::shared_ptr<Transform>;

class TransformAnimation : public Animation
{
public:
    RawVector<TVP<float3>>  translation;
    RawVector<TVP<quatf>>   rotation;
    RawVector<TVP<float3>>  scale;
    RawVector<TVP<bool>>    visibility;

    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

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

    void createAnimation() override;
    void applyScaleFactor(float scale) override;
};
using CameraPtr = std::shared_ptr<Camera>;

class CameraAnimation : public Animation
{
using super = Animation;
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

    void createAnimation() override;
    void applyScaleFactor(float scale) override;
};
using LightPtr = std::shared_ptr<Light>;

class LightAnimation : public Animation
{
using super = Animation;
public:
    RawVector<TVP<float4>>  color;
    RawVector<TVP<float>>   intensity;
    RawVector<TVP<float>>   range;
    RawVector<TVP<float>>   spot_angle; // for spot light

    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    bool empty() const override;
};


// Mesh

struct MeshDataFlags
{
    uint32_t visible : 1;
    uint32_t has_refine_settings : 1;
    uint32_t has_indices : 1;
    uint32_t has_counts : 1;
    uint32_t has_points : 1;
    uint32_t has_normals : 1;
    uint32_t has_tangents : 1;
    uint32_t has_uv : 1;
    uint32_t has_colors : 1;
    uint32_t has_materialIDs : 1;
    uint32_t has_bones : 1;
    uint32_t has_blendshapes : 1;
    uint32_t has_npoints : 1;
    uint32_t apply_trs : 1;
};

struct MeshRefineFlags
{
    uint32_t split : 1;
    uint32_t triangulate : 1;
    uint32_t optimize_topology : 1;
    uint32_t swap_handedness : 1;
    uint32_t swap_faces : 1;
    uint32_t gen_normals : 1;
    uint32_t gen_normals_with_smooth_angle : 1;
    uint32_t gen_tangents : 1;
    uint32_t apply_local2world : 1;
    uint32_t apply_world2local : 1;
    uint32_t bake_skin : 1;
    uint32_t bake_cloth : 1;

    uint32_t invert_v : 1;
    uint32_t mirror_x : 1;
    uint32_t mirror_y : 1;
    uint32_t mirror_z : 1;
    uint32_t mirror_x_weld : 1;
    uint32_t mirror_y_weld : 1;
    uint32_t mirror_z_weld : 1;

    uint32_t gen_weights4 : 1;
};

struct MeshRefineSettings
{
    MeshRefineFlags flags = { 0 };
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f;
    int split_unit = 65000;
    int max_bones_par_vertices = 4;
    float4x4 local2world = float4x4::identity();
    float4x4 world2local = float4x4::identity();
};

struct SubmeshData
{
    IArray<int> indices;
    int materialID = 0;
};

struct SplitData
{
    IArray<float3> points;
    IArray<float3> normals;
    IArray<float4> tangents;
    IArray<float2> uv;
    IArray<float4> colors;
    IArray<int> indices;
    IArray<Weights4> weights4;
    IArray<SubmeshData> submeshes;
};

struct BlendshapeData
{
    std::string name;
    float weight = 0.0f;
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float3> tangents;

    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
using BlendshapeDataPtr = std::shared_ptr<BlendshapeData>;

class Mesh : public Transform
{
using super = Transform;
public:
    MeshDataFlags     flags = { 0 };
    MeshRefineSettings refine_settings;

    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv;
    RawVector<float4> colors;
    RawVector<int>    counts;
    RawVector<int>    indices;
    RawVector<int>    materialIDs;
    RawVector<float3> npoints; // points for normal calculation

    // bone data
    int bones_per_vertex = 0;
    RawVector<float> bone_weights;
    RawVector<int> bone_indices;
    std::vector<std::string> bones;
    RawVector<float4x4> bindposes;

    // blendshape data
    std::vector<BlendshapeDataPtr> blendshape;

    // not serialized
    RawVector<SubmeshData> submeshes;
    RawVector<SplitData> splits;
    RawVector<Weights4> weights4;

public:
    Mesh();
    void clear();
    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    void swapHandedness() override;
    void applyScaleFactor(float scale) override;

    void refine(const MeshRefineSettings& mrs);
    void applyMirror(const float3& plane_n, float plane_d, bool welding = false);
    void applyTransform(const float4x4& t);
    void generateWeights4();
};
using MeshPtr = std::shared_ptr<Mesh>;


enum class Handedness
{
    Left,
    Right,
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

struct Scene
{
public:
    SceneSettings settings;
    std::vector<MeshPtr> meshes;
    std::vector<TransformPtr> transforms;
    std::vector<CameraPtr> cameras;
    std::vector<LightPtr> lights;
    std::vector<MaterialPtr> materials;

public:
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
using ScenePtr = std::shared_ptr<Scene>;




enum class MessageType
{
    Unknown,
    Get,
    Set,
    Delete,
    Fence,
    Text,
    Screenshot,
};

enum class SenderType
{
    Unknown,
    Unity,
    Metasequoia,
};



class Message
{
public:
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
    uint32_t get_uv : 1;
    uint32_t get_colors : 1;
    uint32_t get_indices : 1;
    uint32_t get_materialIDs : 1;
    uint32_t get_bones : 1;
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


std::string ToUTF8(const char *src);
std::string ToUTF8(const std::string& src);
std::string ToANSI(const char *src);
std::string ToANSI(const std::string& src);

} // namespace ms
