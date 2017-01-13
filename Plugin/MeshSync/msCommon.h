#pragma once

#include "MeshUtils/RawVector.h"
#include "MeshUtils/MeshUtils.h"

namespace ms {

enum class EventType
{
    Unknown,
    Get,
    Delete,
    Xform,
    Mesh,
};

union GetFlags
{
    uint32_t flags = 0;
    struct {
        uint32_t get_xforms : 1;
        uint32_t get_meshes : 1;
        uint32_t mesh_get_points : 1;
        uint32_t mesh_get_normals : 1;
        uint32_t mesh_get_tangents : 1;
        uint32_t mesh_get_uv : 1;
        uint32_t mesh_get_indices : 1;
        uint32_t mesh_get_bones : 1;
        uint32_t mesh_swap_handedness : 1;
        uint32_t mesh_swap_faces : 1;
        uint32_t mesh_apply_transform : 1;
    };
};

union MeshRefineFlags
{
    uint32_t flags = 0;
    struct {
        uint32_t split : 1;
        uint32_t gen_normals : 1;
        uint32_t gen_tangents : 1;
        uint32_t swap_handedness : 1;
        uint32_t swap_faces : 1;
    };
};


struct DeleteDataCS;
struct XformDataCS;
struct MeshDataCS;

class EventData
{
public:
    EventType type = EventType::Unknown;

    virtual ~EventData() = 0;
    virtual void clear() = 0;
    virtual uint32_t getSerializeSize() const = 0;
    virtual void serialize(std::ostream& os) const = 0;
    virtual void deserialize(std::istream& is) = 0;
};

class GetData : public EventData
{
public:
    GetFlags flags;

    GetData();
    void clear() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};

class DeleteData : public EventData
{
using super = EventData;
public:
    std::string obj_path;

    DeleteData();
    void clear() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};

class XformData : public EventData
{
using super = EventData;
public:
    std::string obj_path;
    float3   position  = float3::zero();
    quatf    rotation  = quatf::identity();
    float3   scale     = float3::one();
    float4x4 transform = float4x4::identity();

    XformData();
    XformData(const XformDataCS& cs);
    void clear() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};

class MeshData : public EventData
{
using super = EventData;
public:
    std::string obj_path;
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv;
    RawVector<int> counts;
    RawVector<int> indices;
    RawVector<int> indices_triangulated;
    float smooth_angle = 0.0f;

    MeshData();
    MeshData(const MeshDataCS& cs);
    void clear() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    void swap(MeshData& v);
    void refine(MeshRefineFlags flags, float scale);
    void transform(const float4x4& t);
};


struct GetDataCS
{
    GetFlags flags;

    GetDataCS(const GetData& v);
};

struct DeleteDataCS
{
    const char *obj_path = nullptr;

    DeleteDataCS(const DeleteData& v);
};

struct XformDataCS
{
    const char *obj_path = nullptr;
    float3   position;
    quatf    rotation;
    float3   scale;
    float4x4 transform;

    XformDataCS(const XformData& v);
};

struct MeshDataCS
{
    const char *obj_path = nullptr;
    float3 *points = nullptr;
    float3 *normals = nullptr;
    float4 *tangents = nullptr;
    float2 *uv = nullptr;
    int *indices = nullptr;
    int num_points = 0;
    int num_indices = 0;
    float4x4 transform;

    MeshDataCS(const MeshData& v);
};

} // namespace ms
