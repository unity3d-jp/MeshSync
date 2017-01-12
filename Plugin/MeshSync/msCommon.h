#pragma once

#include "MeshUtils/RawVector.h"
#include "MeshUtils/MeshUtils.h"

namespace ms {

enum class EventType
{
    Unknown,
    Delete,
    Xform,
    Mesh,
};

union MeshFlags
{
    uint32_t flags = 0x1 | 0x2;
    struct {
        uint32_t split : 1;
        uint32_t gen_normals : 1;
        uint32_t gen_tangents : 1;
        uint32_t swap_handedness : 1;
        uint32_t swap_faces : 1;
    };
};


struct EventData
{
    EventType type = EventType::Unknown;
    std::string obj_path;
};

struct DeleteData : public EventData
{
    DeleteData();
    void clear();
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};

struct XformData : public EventData
{
    float3 position;
    quatf rotation;
    float3 scale;
    float4x4 transform;

    XformData();
    void clear();
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};

struct MeshData : public EventData
{
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv;
    RawVector<int> counts;
    RawVector<int> indices;
    RawVector<int> indices_triangulated;
    float smooth_angle = 0.0f;

    MeshData();
    void clear();
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);

    void refine(MeshFlags flags, float scale);
};



struct DeleteDataCS
{
    const char *obj_path = nullptr;

    DeleteDataCS(const DeleteData& v);
};

struct XformDataCS
{
    const char *obj_path = nullptr;
    float3 position;
    quatf rotation;
    float3 scale;
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

    MeshDataCS(const MeshData& v);
};

} // namespace ms
