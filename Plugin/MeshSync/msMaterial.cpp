#include "pch.h"
#include "msSceneGraph.h"
#include "msMaterial.h"
#include "msSceneGraphImpl.h"


namespace ms {

Texture * Texture::make(std::istream & is)
{
    auto ret = new Texture();
    ret->deserialize(is);
    return ret;
}

#define EachMember(F)\
    F(id) F(filename) F(type) F(data)

uint32_t Texture::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(Size);
    return ret;
}

void Texture::serialize(std::ostream & os) const
{
    EachMember(Write);
}

void Texture::deserialize(std::istream & is)
{
    EachMember(Read);
}

#undef EachMember


Material * Material::make(std::istream & is)
{
    auto ret = new Material();
    ret->deserialize(is);
    return ret;
}

#define EachMember(F)\
    F(id) F(name) F(color) F(emission) F(metalic) F(smoothness) F(opacity)

uint32_t Material::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(Size);
    return ret;
}
void Material::serialize(std::ostream& os) const
{
    EachMember(Write);
}
void Material::deserialize(std::istream& is)
{
    EachMember(Read);
}

#undef EachMember

} // namespace ms
