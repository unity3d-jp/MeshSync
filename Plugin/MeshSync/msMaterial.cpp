#include "pch.h"
#include "msMaterial.h"
#include "msSceneGraphImpl.h"


namespace ms {

std::shared_ptr<Texture> Texture::create(std::istream & is)
{
    auto ret = Pool<Texture>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Texture::Texture() {}
Texture::~Texture() {}

#define EachMember(F)\
    F(id) F(type) F(filename) F(data)

uint32_t Texture::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void Texture::serialize(std::ostream & os) const
{
    EachMember(msWrite);
}

void Texture::deserialize(std::istream & is)
{
    EachMember(msRead);
}

void Texture::clear()
{
    id = 0;
    type = TextureType::Default;
    filename.clear();
    data.clear();
}

#undef EachMember


std::shared_ptr<Material> Material::create(std::istream & is)
{
    auto ret = Pool<Material>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Material::Material() {}
Material::~Material() {}

#define EachMember(F)\
    F(id) F(name) F(color) F(emission) F(metalic) F(smoothness)

uint32_t Material::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void Material::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void Material::deserialize(std::istream& is)
{
    EachMember(msRead);
}

void Material::clear()
{
    id = 0;
    name.clear();

    color = float4::one();
    emission = float4::zero();
    metalic = 0.0f;
    smoothness = 0.5f;
}

bool Material::operator==(const Material& v) const
{
    return
        name == v.name &&
        color == v.color &&
        emission == v.emission &&
        metalic == v.metalic &&
        smoothness == v.smoothness;
}
bool Material::operator!=(const Material& v) const
{
    return !(*this == v);
}

#undef EachMember

} // namespace ms
