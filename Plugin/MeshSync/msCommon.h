#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include "MeshUtils/RawVector.h"
#include "MeshUtils/MeshUtils.h"

namespace ms {

void LogImpl(const char *fmt, ...);

#define msLogInfo(...)    ::ms::LogImpl("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::ms::LogImpl("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::ms::LogImpl("MeshSync error: " __VA_ARGS__)


enum class MessageType
{
    Unknown,
    Get,
    Delete,
    Mesh,
    Screenshot,
};

enum class SenderType
{
    Unknown,
    Unity,
    Metasequoia,
};



class MessageData
{
public:
    virtual ~MessageData();
    virtual uint32_t getSerializeSize() const = 0;
    virtual void serialize(std::ostream& os) const = 0;
    virtual void deserialize(std::istream& is) = 0;
};


// Get request

struct GetFlags
{
    uint32_t get_transform : 1;
    uint32_t get_points : 1;
    uint32_t get_normals : 1;
    uint32_t get_tangents : 1;
    uint32_t get_uv : 1;
    uint32_t get_indices : 1;
    uint32_t get_materialIDs : 1;
    uint32_t get_bones : 1;
    uint32_t bake_skin : 1;
    uint32_t swap_handedness : 1;
    uint32_t swap_faces : 1;
    uint32_t apply_local2world : 1;
    uint32_t apply_world2local : 1;
    uint32_t invert_v : 1;
};

class GetData : public MessageData
{
public:
    GetFlags flags = {0};
    float scale = 1.0f;
    std::shared_ptr<std::atomic_int> wait_flag;

    GetData();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};

class ScreenshotData : public MessageData
{
public:
    std::shared_ptr<std::atomic_int> wait_flag;

    ScreenshotData();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};


// Delete request

class DeleteData : public MessageData
{
using super = MessageData;
public:
    std::string path;
    int id = 0;

    DeleteData();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};


// Mesh

struct MeshDataFlags
{
    uint32_t visible : 1;
    uint32_t has_indices : 1;
    uint32_t has_counts : 1;
    uint32_t has_points : 1;
    uint32_t has_normals : 1;
    uint32_t has_tangents : 1;
    uint32_t has_uv : 1;
    uint32_t has_materialIDs : 1;
    uint32_t has_transform : 1;
    uint32_t has_refine_settings : 1;
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

    // Metasequoia - equivalent
    uint32_t invert_v : 1;
    uint32_t mirror_x : 1;
    uint32_t mirror_y : 1;
    uint32_t mirror_z : 1;
};

struct MeshRefineSettings
{
    MeshRefineFlags flags = { 0 };
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f;
    int split_unit = 65000;
};

struct Transform
{
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   rotation_eularZXY = float3::zero();
    float3   scale = float3::one();
    float4x4 local2world = float4x4::identity();
    float4x4 world2local = float4x4::identity();
};

struct SubmeshData
{
    IntrusiveArray<int> indices;
    int materialID = 0;
};

struct SplitData
{
    IntrusiveArray<float3> points;
    IntrusiveArray<float3> normals;
    IntrusiveArray<float4> tangents;
    IntrusiveArray<float2> uv;
    IntrusiveArray<int> indices;
    IntrusiveArray<SubmeshData> submeshes;
};

class MeshData : public MessageData
{
using super = MessageData;
public:
    SenderType        sender = SenderType::Unknown;
    int               id = 0;
    std::string       path;
    MeshDataFlags     flags = {0};
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv;
    RawVector<int>    counts;
    RawVector<int>    indices;
    RawVector<int>    materialIDs;

    Transform         transform;
    MeshRefineSettings refine_settings;

    // not serialized
    RawVector<SubmeshData> submeshes;
    RawVector<SplitData> splits;


    MeshData();
    void clear();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    const char* getName() const;
    void refine();
    void applyMirror(const float3& plane_n, float plane_d);
    void applyTransform(const float4x4& t);
};

} // namespace ms
