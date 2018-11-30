#include "pch.h"
#include "msIdentifier.h"

namespace ms {

// Identifier
Identifier::Identifier() {}
Identifier::Identifier(const std::string& p, int i) : name(p), id(i) {}

void Identifier::serialize(std::ostream& os) const
{
    write(os, name); write(os, id);
}
void Identifier::deserialize(std::istream& is)
{
    read(is, name); read(is, id);
}

} // namespace ms
