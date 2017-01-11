#pragma once

#include "RawVector.h"

namespace ms {

using namespace mu;

enum class EventType
{
    Unknown,
    Edit,
};


struct EventData
{
    EventType type = EventType::Unknown;
};

struct EditData : public EventData
{
    std::string obj_path;
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv;
    RawVector<int> indices;
    float3 position{ 0.0f, 0.0f, 0.0f };

    EditData();
    void clear();
    void generateNormals(bool gen_tangents);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    bool deserialize(std::istream& is);
};

struct EditDataRef
{
    const char *obj_path = nullptr;
    float3 *points = nullptr;
    float3 *normals = nullptr;
    float4 *tangents = nullptr;
    float2 *uv = nullptr;
    int *indices = nullptr;
    int num_points = 0;
    int num_indices = 0;

    float3 position{ 0.0f, 0.0f, 0.0f };

    EditDataRef& operator=(const EditData& v);
};

} // namespace ms
