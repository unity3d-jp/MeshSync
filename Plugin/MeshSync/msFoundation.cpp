#include "pch.h"
#include "msFoundation.h"
#include "msSceneGraphImpl.h"

namespace ms {

// Identifier
Identifier::Identifier() {}
Identifier::Identifier(const std::string& p, int i) : name(p), id(i) {}

uint32_t Identifier::getSerializeSize() const
{
    return ssize(name) + ssize(id);
}
void Identifier::serialize(std::ostream& os) const
{
    write(os, name); write(os, id);
}
void Identifier::deserialize(std::istream& is)
{
    read(is, name); read(is, id);
}

} // namespace ms
