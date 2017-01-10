#pragma once

namespace ms {

using namespace mu;

enum class EventType
{
    Unknown,
    Edit,
};

struct EditEventData
{
    const char *obj_path = nullptr;
    const float3 *points = nullptr;
    const float2 *uv = nullptr;
    const int *indices = nullptr;
    int num_points = 0;
    int num_indices = 0;

    float3 position{0.0f, 0.0f, 0.0f};
};


struct EditData
{
    std::string obj_path;
    std::vector<float3> points;
    std::vector<float2> uv;
    std::vector<int> indices;
    float3 position{ 0.0f, 0.0f, 0.0f };

    void clear();
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    bool deserialize(std::istream& is);
};

} // namespace ms
