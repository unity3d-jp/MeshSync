#pragma once
#include "msFoundation.h"

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

} // namespace ms
