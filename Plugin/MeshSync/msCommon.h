#pragma once

#include "RawVector.h"
#include "MeshUtils/MeshUtils.h"

namespace ms {

enum class EventType
{
    Unknown,
    Delete,
    Xform,
    Mesh,
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

    MeshData();
    void clear();
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);

    void generateNormals(bool gen_tangents);
};



struct DeleteDataRef
{
    const char *obj_path = nullptr;

    DeleteDataRef(const DeleteData& v);
};

struct XformDataRef
{
    const char *obj_path = nullptr;
    float3 position;
    quatf rotation;
    float3 scale;
    float4x4 transform;

    XformDataRef(const XformData& v);
};

struct MeshDataRef
{
    const char *obj_path = nullptr;
    float3 *points = nullptr;
    float3 *normals = nullptr;
    float4 *tangents = nullptr;
    float2 *uv = nullptr;
    int *counts = nullptr;
    int *indices = nullptr;
    int num_points = 0;
    int num_indices = 0;

    MeshDataRef(const MeshData& v);
};

} // namespace ms
