#pragma once

#include "MeshSync/msFoundation.h" //msSerializable

namespace ms {

const int InvalidID = -1;

struct Identifier
{
    std::string name;
    int id = InvalidID;

    Identifier();
    Identifier(const std::string& p, int i);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(Identifier);


struct Bounds
{
    mu::float3 center;
    mu::float3 extents;

    bool operator==(const Bounds& v) const { return center == v.center && extents == v.extents; }
    bool operator!=(const Bounds& v) const { return !(*this == v); }
};

} // namespace ms
